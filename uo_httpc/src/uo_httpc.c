#include "uo_httpc.h"
#include "uo_err.h"
#include "uo_mem.h"

#include <openssl/ssl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>
#include <tchar.h>

static X509_STORE *x509_store;
#endif

#define RECV_RETRY_MAX_TRIES 3
#define RECV_RETRY_WAIT_SLOT_USEC 0x10000
#define RECV_TIMEOUT_SEC 20
#define SEND_TIMEOUT_SEC 20

#define BUF_INIT_LEN 0x10000

#define STRLEN(s) (sizeof(s) / sizeof(s[0]) - 1)

#define CRLF "\r\n"

#define HTTP_GET "GET "
#define HTTP_POST "POST "
#define HTTP_PUT "PUT "
#define HTTP_1_1 " HTTP/1.1"

#define HEADER_HOST "Host: "
#define HEADER_ACCEPT "Accept: "
#define HEADER_CONNECTION "Connection: "
#define HEADER_CONTENT_LENGTH "Content-Length: "
#define HEADER_CONTENT_TYPE "Content-Type: "

#define HEADER_TRANSFER_ENCODING_CHUNKED "Transfer-Encoding: chunked"

typedef struct uo_tls_info {
    SSL_CTX *ctx;
} uo_tls_info;

static bool is_init;

uo_http_res *uo_http_res_create(
    int status_code,
    const char *headers,
    const size_t headers_len, 
    const char *body, 
    const size_t body_len);

/* returns
    == -1 : no matches, possibly error
    >= 0  : Content-Length: <value>
    == -2 : Transfer-Encoding: chunked */
static long parse_response_headers(
    const char *const response,
    size_t len)
{
    const char *response_end = response + len;

    const char *header = response;
    const char *header_end = memchr(header, '\r', response_end - header);
    
    while (header_end && header_end < response_end)
    {
        // Find Transfer-Encoding: chunked header using bitwise trick to do case insensitive comparison
        if ((((uint64_t *)header)[0] & 0xDFDFDFDFDFDFDFDF) == (((uint64_t *)HEADER_TRANSFER_ENCODING_CHUNKED)[0] & 0xDFDFDFDFDFDFDFDF) && 
            (((uint64_t *)header)[1] & 0xDFDFDFDFDFDFDFDF) == (((uint64_t *)HEADER_TRANSFER_ENCODING_CHUNKED)[1] & 0xDFDFDFDFDFDFDFDF) && 
            (((uint64_t *)header)[2] & 0xDFDFDFDFDFDFDFDF) == (((uint64_t *)HEADER_TRANSFER_ENCODING_CHUNKED)[2] & 0xDFDFDFDFDFDFDFDF) &&
            (((uint64_t *)header)[3] & 0x000000000000DFDF) == (((uint64_t *)HEADER_TRANSFER_ENCODING_CHUNKED)[3] & 0x000000000000DFDF))
            return -2;

        // Find Content-Length header using bitwise trick to do case insensitive comparison
        if ((((uint64_t *)header)[0] & 0xDFDFDFDFDFDFDFDF) == (((uint64_t *)HEADER_CONTENT_LENGTH)[0] & 0xDFDFDFDFDFDFDFDF) && 
            (((uint64_t *)header)[1] & 0xDFDFDFDFDFDFDFDF) == (((uint64_t *)HEADER_CONTENT_LENGTH)[1] & 0xDFDFDFDFDFDFDFDF))
        {
            long content_length;
            const char *header_val = header + STRLEN(HEADER_CONTENT_LENGTH);
            return sscanf(header_val, "%ld", &content_length) == 1
                ? content_length
                : -1;
        }

        header = header_end + STRLEN(CRLF);
        header_end = memchr(header, '\r', response_end - header);
    }
    
    return -1;
}

static int expbackoff_usleep(
    const uint32_t c, 
    const useconds_t slot_usec)
{
    const uint32_t k = rand() % (1 << c);
    return usleep(k * slot_usec);
}

static int create_tcp_socket(
    int ai_family)
{
    int sockfd = socket(ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
       return sockfd;

    int opt_TCP_NODELAY = true;
    if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
        uo_err("Could not set TCP_NODELAY option.");

    struct timeval opt_SO_RCVTIMEO = { .tv_sec = RECV_TIMEOUT_SEC };
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO) == -1)
        uo_err("Could not set SO_RCVTIMEO option.");

    struct timeval opt_SO_SNDTIMEO = { .tv_sec = SEND_TIMEOUT_SEC };
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO) == -1)
        uo_err("Could not set SO_SNDTIMEO option.");

    return sockfd;
}

static bool uo_httpc_grow_buf(
    uo_httpc *httpc)
{
    ptrdiff_t hostnamediff = httpc->hostname - httpc->buf;
    httpc->buf = realloc(httpc->buf, httpc->buf_len <<= 1);
    httpc->hostname = httpc->buf + hostnamediff;

    return !!httpc->buf;
}

static bool uo_httpc_set_tls_info(
    uo_httpc *httpc)
{
    uo_tls_info *tls_info = malloc(sizeof *tls_info);

    const SSL_METHOD *method = TLS_client_method();
    if (!method)
        uo_err_return(false, "Error creating SSL_METHOD.");

    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx)
        uo_err_return(false, "Error creating SSL_CTX.");

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 4);
    SSL_CTX_set_min_proto_version(ctx, TLS1_VERSION);
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    const char *const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
    if (!SSL_CTX_set_cipher_list(ctx, PREFERRED_CIPHERS))
        uo_err_goto(err_free_ctx, "Error while setting cipher list for TLS.");

#ifdef WIN32
    SSL_CTX_set_cert_store(ctx, x509_store);
#else
    if (!SSL_CTX_set_default_verify_paths(ctx))
        uo_err_goto(err_free_ctx, "Error setting CA certificate locations for HTTP client.");
#endif

    tls_info->ctx = ctx;
    httpc->tls_info = tls_info;

    return true;

err_free_ctx:
    SSL_CTX_free(ctx);

    return false;
}

static uo_http_res *uo_httpc_make_request(
    uo_httpc *httpc,
    uo_cb *_)
{
    const uo_tls_info *const tls_info = httpc->tls_info;
    SSL *ssl = NULL;

    char *request = httpc->buf + httpc->headers_len;

    int sockfd = create_tcp_socket(httpc->serv_addrinfo->ai_family);
    if (sockfd == -1)
        uo_err_return(NULL, "Unable to create socket.");

    if (tls_info)
    {
        if (!(ssl = SSL_new(tls_info->ctx)))
            uo_err_return(NULL, "Unable to setup TLS.");

        if (!SSL_set_fd(ssl, sockfd))
            uo_err_goto(err_ssl_free, "Unable to setup TLS.");

        uo_mem_using(hostname, httpc->hostname_len + 1)
        {
            char *p = hostname;
            uo_mem_write(p, httpc->hostname, httpc->hostname_len);
            *p = '\0';

            if (!SSL_set1_host(ssl, hostname)) 
                uo_err_goto(err_ssl_free, "Unable to setup TLS.");

            if (!SSL_set_tlsext_host_name(ssl, hostname))
                uo_err_goto(err_ssl_free, "Unable to setup TLS.");
        }
    }

    if (connect(sockfd, httpc->serv_addrinfo->ai_addr, httpc->serv_addrinfo->ai_addrlen) == -1)
        uo_err_goto(err_close, "Unable to connect HTTP client socket.");

    if (tls_info)
    {
        if (SSL_connect(ssl) != 1)
            uo_err_goto(err_close, "Error while trying to connect using TLS.");

        if (SSL_do_handshake(ssl) != 1)
            uo_err_goto(err_close, "Error while conducting the TLS handshake.");

        X509 *cert = SSL_get_peer_certificate(ssl);
        if (!cert)
            uo_err_goto(err_close, "Error getting the peer certificate.");
        
        X509_free(cert);

        if (SSL_get_verify_result(ssl) != X509_V_OK)
            uo_err_goto(err_close, "Error verifying the peer certificate.");

        if (SSL_write(ssl, request, httpc->request_len) != httpc->request_len)
            uo_err_goto(err_close, "Error while sending the HTTP request.");
    }
    else if (send(sockfd, request, httpc->request_len, 0) == -1)
            uo_err_goto(err_close, "Error on sending HTTP request.");

    char *response = request + httpc->request_len;
    char *headers_end = NULL;
    char *p = response;

    size_t response_buf_len = httpc->buf_len - (response - httpc->buf);
    ssize_t recv_len;

    uint32_t recv_retries = 0;

    // Read headers
    do
    {
        while ((recv_len = (tls_info ? SSL_read(ssl, p, response_buf_len) : recv(sockfd, p, response_buf_len, 0))) == -1)
        {
            int errno_local = errno;
            if ((errno_local == EAGAIN || errno_local == EWOULDBLOCK) && recv_retries < RECV_RETRY_MAX_TRIES)
                expbackoff_usleep(++recv_retries, RECV_RETRY_WAIT_SLOT_USEC);
            else
                uo_err_goto(err_close, "Error on receiving HTTP response.");
        }

        p += recv_len;

        if (!(response_buf_len = httpc->buf_len - (p - httpc->buf)))
        {
            ptrdiff_t pdiff = p - httpc->buf;
            ptrdiff_t responsediff = response - httpc->buf;

            if (!uo_httpc_grow_buf(httpc))
                uo_err_goto(err_close, "Error reallocating the buffer.");

            p = httpc->buf + pdiff;
            response = httpc->buf + responsediff;
        }
            
        *p = '\0';

    }  while (recv_len && !(headers_end = strstr(response, CRLF CRLF)));

    long lenflags = parse_response_headers(response, p - response);
    char *body = headers_end + STRLEN(CRLF CRLF);

    switch (lenflags)
    {
        case -2: /* Transfer-Encoding: chunked */
        {
            size_t chunk_size = 1;
            char *chunk = body;
            char *chunk_data;

            do
            {
                while(p > chunk && (chunk_data = strstr(chunk, CRLF)))
                {
                    chunk_data += STRLEN(CRLF);
                    sscanf(chunk, "%lx", &chunk_size);
                    if (chunk == body)
                    {
                        memmove(chunk, chunk_data, p - chunk);
                        p -= chunk_data - chunk;
                        chunk += chunk_size + STRLEN(CRLF);
                    }
                    else
                    {
                        memmove(chunk - STRLEN(CRLF), chunk_data, p - chunk);
                        p -= chunk_data - chunk + STRLEN(CRLF);
                        chunk += chunk_size;
                    }
                }

                if (!chunk_size)
                    break;

                while ((recv_len = (tls_info ? SSL_read(ssl, p, response_buf_len) : recv(sockfd, p, response_buf_len, 0))) == -1)
                {
                    int errno_local = errno;
                    if ((errno_local == EAGAIN || errno_local == EWOULDBLOCK) && recv_retries < RECV_RETRY_MAX_TRIES)
                        expbackoff_usleep(++recv_retries, RECV_RETRY_WAIT_SLOT_USEC);
                    else
                        uo_err_goto(err_close, "Error on receiving HTTP response.");
                }

                p += recv_len;

                if (!(response_buf_len = httpc->buf_len - (p - httpc->buf)))
                {
                    ptrdiff_t pdiff = p - httpc->buf;
                    ptrdiff_t responsediff = response - httpc->buf;
                    ptrdiff_t headers_enddiff = headers_end - httpc->buf;
                    ptrdiff_t chunkdiff = chunk - httpc->buf;

                    if (!uo_httpc_grow_buf(httpc))
                        uo_err_goto(err_close, "Error reallocating the buffer.");

                    p = httpc->buf + pdiff;
                    response = httpc->buf + responsediff;
                    headers_end = httpc->buf + headers_enddiff;
                    chunk = httpc->buf + chunkdiff;
                }

                *p = '\0';

            } while (recv_len);

            break;
        }

        case -1: /* stop reading on error or when no more data */
        {
            while ((recv_len = (tls_info ? SSL_read(ssl, p, response_buf_len) : recv(sockfd, p, response_buf_len, 0))) > 0)
            {
                p += recv_len;

                if (!(response_buf_len = httpc->buf_len - (p - httpc->buf)))
                {
                    ptrdiff_t pdiff = p - httpc->buf;
                    ptrdiff_t responsediff = response - httpc->buf;
                    ptrdiff_t headers_enddiff = headers_end - httpc->buf;

                    if (!uo_httpc_grow_buf(httpc))
                        uo_err_goto(err_close, "Error reallocating the buffer.");

                    p = httpc->buf + pdiff;
                    response = httpc->buf + responsediff;
                    headers_end = httpc->buf + headers_enddiff;
                }
                    
                *p = '\0';
            }

            break;
        }

        default: /* Content-Length: <lenflags> */
        {
            while (recv_len && p - body < lenflags)
            {
                while ((recv_len = (tls_info ? SSL_read(ssl, p, response_buf_len) : recv(sockfd, p, response_buf_len, 0))) == -1)
                {
                    int errno_local = errno;
                    if ((errno_local == EAGAIN || errno_local == EWOULDBLOCK) && recv_retries < RECV_RETRY_MAX_TRIES)
                        expbackoff_usleep(++recv_retries, RECV_RETRY_WAIT_SLOT_USEC);
                    else
                        uo_err_goto(err_close, "Error on receiving HTTP response.");
                }

                p += recv_len;

                if (!(response_buf_len = httpc->buf_len - (p - httpc->buf)))
                {
                    ptrdiff_t pdiff = p - httpc->buf;
                    ptrdiff_t responsediff = response - httpc->buf;
                    ptrdiff_t headers_enddiff = headers_end - httpc->buf;

                    if (!uo_httpc_grow_buf(httpc))
                        uo_err_goto(err_close, "Error reallocating the buffer.");

                    p = httpc->buf + pdiff;
                    response = httpc->buf + responsediff;
                    headers_end = httpc->buf + headers_enddiff;
                }
                    
                *p = '\0';
            }

            break;
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    if (tls_info)
        SSL_free(ssl);

    int status_code;
    if (p - response < 12 || !sscanf(response, "HTTP/1.%*c %d", &status_code))
        uo_err_return(NULL, "Error on receiving the HTTP response.");
    
    return p > body
        ? uo_http_res_create(
            status_code,
            response,
            headers_end - response,
            body,
            p - body)
        : uo_http_res_create(
            status_code,
            response,
            headers_end - response,
            NULL,
            0);

err_close:
    close(sockfd);

err_ssl_free:
    if (tls_info)
        SSL_free(ssl);

    return NULL;
}

static void *uo_httpc_init_request(
    uo_httpc *httpc,
    const char *method,
    size_t method_len,
    const char *path,
    size_t path_len,
    const char *body,
    size_t body_len,
    uo_cb *uo_http_res_cb)
{
    while (httpc->buf_len < body_len + httpc->headers_len * 2 + 0x400)
        uo_httpc_grow_buf(httpc);

    char *const request = httpc->buf + httpc->headers_len;

    char *p = request;
    uo_mem_write(p, method, method_len);
    uo_mem_write(p, path, path_len);
    uo_mem_write(p, HTTP_1_1, STRLEN(HTTP_1_1));
    uo_mem_write(p, CRLF, STRLEN(CRLF));
    uo_mem_write(p, httpc->buf, httpc->headers_len);

    if (body)
    {
        uo_mem_write(p, HEADER_CONTENT_LENGTH, STRLEN(HEADER_CONTENT_LENGTH));
        p += sprintf(p, "%lu", body_len);
        uo_mem_write(p, CRLF CRLF, STRLEN(CRLF) * 2);
        uo_mem_write(p, body, body_len);
    }
    else
    {
        uo_mem_write(p, CRLF, STRLEN(CRLF));
    }

    httpc->request_len = p - request;

    uo_cb_prepend(uo_http_res_cb, (void *(*)(void *, uo_cb *))uo_httpc_make_request);

    sem_t *sem = httpc->opt & UO_HTTPC_OPT_SEM
        ? malloc(sizeof *sem)
        : NULL;

    uo_cb_invoke_async(uo_http_res_cb, httpc, sem);

    return sem;
}

bool uo_httpc_init(
    size_t thrd_count)
{
    if (is_init)
    {
        is_init &= uo_cb_init();
        is_init &= uo_sock_init();
        return is_init;
    }

    is_init = true;
    is_init &= uo_cb_init();
    is_init &= uo_sock_init();

    if (OPENSSL_init_ssl(0, NULL) != 1)
        uo_err_return(is_init = false, "Initialization of OpenSSL failed");

    srand(time(0));

#ifdef WIN32

    x509_store = X509_STORE_new();

    PCCERT_CONTEXT pContext = NULL;
    HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");

    if (!hStore)
        uo_err_return(is_init = false, "Initialization of OpenSSL failed");

    while (pContext = CertEnumCertificatesInStore(hStore, pContext))
    {
        X509 *x509 = d2i_X509(NULL, (const unsigned char **)&pContext->pbCertEncoded, pContext->cbCertEncoded);
        if (x509 && X509_STORE_add_cert(x509_store, x509) == 1)
            X509_free(x509);
        else
            uo_err_return(is_init = false, "Initialization of OpenSSL failed");
    }

    CertCloseStore(hStore, 0);

#endif

    return is_init;
}

uo_httpc *uo_httpc_create(
    const char *hostname, 
    size_t hostname_len,
    UO_HTTPC_OPT opt)
{
    uo_httpc *httpc = malloc(sizeof *httpc);

    httpc->hostname_len = hostname_len;
    httpc->opt = opt;

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    int gai = getaddrinfo(
        hostname, 
        opt & UO_HTTPC_OPT_TLS ? "443" : "80",
        &hints, 
        &httpc->serv_addrinfo);
    
    if (gai != 0)
        uo_err_goto(err_free, "getaddrinfo: %s", gai_strerror(gai));

    httpc->header_flags = HTTP_HEADER_HOST;
    httpc->headers_len = STRLEN(HEADER_HOST) + hostname_len + STRLEN(CRLF);

    httpc->buf_len = BUF_INIT_LEN > httpc->headers_len
        ? BUF_INIT_LEN
        : httpc->headers_len;

    httpc->buf = malloc(httpc->buf_len);
    httpc->hostname = httpc->buf + STRLEN(HEADER_HOST);

    void *p = httpc->buf;
    uo_mem_write(p, HEADER_HOST, STRLEN(HEADER_HOST));
    uo_mem_write(p, hostname, hostname_len);
    uo_mem_write(p, CRLF, STRLEN(CRLF));

    for (size_t i = 0; i < hostname_len; ++i)
        httpc->hostname[i] = tolower(httpc->hostname[i]);

    assert(p - (void *)httpc->buf == httpc->headers_len);

    if (opt & UO_HTTPC_OPT_TLS)
    {
        if (!uo_httpc_set_tls_info(httpc))
            uo_err_goto(err_free, "Unable to setup TLS for HTTP client.");
    }
    else
        httpc->tls_info = NULL;

    return httpc;

err_free:
    free(httpc);

    return NULL;
}

void uo_httpc_destroy(
    uo_httpc *httpc)
{
    freeaddrinfo(httpc->serv_addrinfo); 
    free(httpc->buf);
    free(httpc);
}

void uo_httpc_set_header(
    uo_httpc *httpc, 
    HTTP_HEADER_FLAGS header, 
    const char *value, 
    size_t value_len)
{
    switch (header)
    {
        case HTTP_HEADER_ACCEPT:
        case HTTP_HEADER_CONNECTION:
        case HTTP_HEADER_CONTENT_TYPE:
            break;
        default:
            return;
    }

    // only set header if not already set
    if ((httpc->header_flags & header) != header)
    {
        httpc->header_flags |= header;

        switch (header)
        {
            case HTTP_HEADER_ACCEPT:
            {

                void *p = httpc->buf + httpc->headers_len;

                httpc->headers_len += STRLEN(HEADER_ACCEPT) + value_len + STRLEN(CRLF);

                while (httpc->buf_len < httpc->headers_len)
                    uo_httpc_grow_buf(httpc);

                uo_mem_write(p, HEADER_ACCEPT, STRLEN(HEADER_ACCEPT));
                uo_mem_write(p, value, value_len);
                uo_mem_write(p, CRLF, STRLEN(CRLF));

                assert(p - (void *)httpc->buf == httpc->headers_len);

                break;
            }

            case HTTP_HEADER_CONNECTION:
            {

                void *p = httpc->buf + httpc->headers_len;

                httpc->headers_len += STRLEN(HEADER_CONNECTION) + value_len + STRLEN(CRLF);

                while (httpc->buf_len < httpc->headers_len)
                    uo_httpc_grow_buf(httpc);

                uo_mem_write(p, HEADER_CONNECTION, STRLEN(HEADER_CONNECTION));
                uo_mem_write(p, value, value_len);
                uo_mem_write(p, CRLF, STRLEN(CRLF));

                assert(p - (void *)httpc->buf == httpc->headers_len);

                break;
            }

            case HTTP_HEADER_CONTENT_TYPE:
            {

                void *p = httpc->buf + httpc->headers_len;

                httpc->headers_len += STRLEN(HEADER_CONTENT_TYPE) + value_len + STRLEN(CRLF);

                while (httpc->buf_len < httpc->headers_len)
                    uo_httpc_grow_buf(httpc);

                uo_mem_write(p, HEADER_CONTENT_TYPE, STRLEN(HEADER_CONTENT_TYPE));
                uo_mem_write(p, value, value_len);
                uo_mem_write(p, CRLF, STRLEN(CRLF));

                assert(p - (void *)httpc->buf == httpc->headers_len);

                break;
            }
        }
    }
}

void *uo_httpc_get(
    uo_httpc *httpc,
    const char *path,
    size_t path_len,
    uo_cb *uo_http_res_cb)
{
    return uo_httpc_init_request(
        httpc, 
        HTTP_GET, 
        STRLEN(HTTP_GET),
        path,
        path_len,
        NULL,
        0,
        uo_http_res_cb);
}

void *uo_httpc_post(
    uo_httpc *httpc,
    const char *path,
    size_t path_len,
    const char *body,
    size_t body_len,
    uo_cb *uo_http_res_cb)
{
    return uo_httpc_init_request(
        httpc, 
        HTTP_POST, 
        STRLEN(HTTP_POST),
        path,
        path_len,
        body,
        body_len,
        uo_http_res_cb);
}

void *uo_httpc_put(
    uo_httpc *httpc,
    const char *path,
    size_t path_len,
    const char *body,
    size_t body_len,
    uo_cb *uo_http_res_cb)
{
    return uo_httpc_init_request(
        httpc, 
        HTTP_PUT, 
        STRLEN(HTTP_PUT),
        path,
        path_len,
        body,
        body_len,
        uo_http_res_cb);
}
