#include "uo_httpc.h"
#include "uo_cb.h"
#include "uo_sock.h"
#include "uo_err.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <pthread.h>

#define BUF_INIT_LEN 0x10000

#define STRLEN(s) (sizeof(s) / sizeof(s[0]) - 1)

#define CRLF "\r\n"

#define HTTP_GET "GET "
#define HTTP_1_1 " HTTP/1.1"

#define HEADER_HOST "Host: "
#define HEADER_ACCEPT "Accept: "
#define HEADER_CONTENT_LENGTH "Content-Length: "

static bool is_init;

uo_http_res *uo_http_res_create(
    int status_code,
    const char *headers,
    const size_t headers_len, 
    const char *body, 
    const size_t body_len);

static uo_http_res *uo_httpc_make_request(
    uo_httpc *httpc,
    void *_)
{
    int sockfd = socket(httpc->serv_addrinfo->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
        uo_err_return(NULL, "Unable to create socket.");

    int opt_TCP_NODELAY = true;
    if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
        uo_err("Could not set TCP_NODELAY option.");

    struct timeval opt_SO_RCVTIMEO = { .tv_sec = 20 };
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO) == -1)
        uo_err("Could not set SO_RCVTIMEO option.");

    struct timeval opt_SO_SNDTIMEO = { .tv_sec = 20 };
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO) == -1)
        uo_err("Could not set SO_SNDTIMEO option.");

    if (connect(sockfd, httpc->serv_addrinfo->ai_addr, httpc->serv_addrinfo->ai_addrlen) == -1)
        uo_err_goto(err_close, "Unable to connect http client socket.");

    char *request = httpc->buf + httpc->headers_len;

    if (send(sockfd, request, httpc->request_len, 0) == -1)
        uo_err_goto(err_close, "Error on sending http request.");

    if (shutdown(sockfd, SHUT_WR) == -1)
        uo_err_goto(err_close, "Error on shutting down the socket for sending.");

    char *response = request + httpc->request_len;
    size_t response_buf_len = httpc->buf_len - (response - httpc->buf);

    char *p = response;
    ssize_t recv_len = ~0;

    while (recv_len)
    {
        p += recv_len = recv(sockfd, p, response_buf_len, 0);

        if (recv_len == -1)
            uo_err_goto(err_close, "Error on receiving http response.");

        if (!(response_buf_len = httpc->buf_len - (p - httpc->buf)))
        {
            ptrdiff_t pdiff = p - httpc->buf;
            ptrdiff_t responsediff = response - httpc->buf;

            httpc->buf = realloc(httpc->buf, httpc->buf_len <<= 1);

            p = httpc->buf + pdiff;
            response = httpc->buf + responsediff;
        }
            
        *p = '\0';
    }

    shutdown(sockfd, SHUT_RD);
    close(sockfd);

    int status_code;
    if (p - response < 12 || !sscanf(response, "HTTP/1.%*c %d", &status_code))
        uo_err_return(NULL, "Error on receiving the HTTP response.");

    char *headers_end = strstr(response, CRLF CRLF);
    if (!headers_end)
        uo_err_return(NULL, "Error on receiving the HTTP response. HTTP header fields were partially missing.");

    char *body = headers_end + STRLEN(CRLF CRLF);
    
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

    return NULL;
}

bool uo_httpc_init(
    size_t thrd_count)
{
    if (is_init)
    {
        is_init &= uo_cb_init(thrd_count);
        is_init &= uo_sock_init();
        return is_init;
    }

    is_init = true;
    is_init &= uo_cb_init(thrd_count);
    is_init &= uo_sock_init();

    return is_init;
}

uo_httpc *uo_httpc_create(
    const char *host, 
    const size_t host_len)
{
    uo_httpc *httpc = malloc(sizeof *httpc);

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    int s;
    
    if ((s = getaddrinfo(host, "80", &hints, &httpc->serv_addrinfo)) != 0)
    {
        free(httpc);
        uo_err_return(NULL, "getaddrinfo: %s", gai_strerror(s));
    }

    httpc->header_flags = HTTP_HEADER_HOST;
    httpc->headers_len = STRLEN(HEADER_HOST) + host_len + STRLEN(CRLF);

    httpc->buf_len = BUF_INIT_LEN > httpc->headers_len
        ? BUF_INIT_LEN
        : httpc->headers_len;

    httpc->buf = malloc(httpc->buf_len);

    void *p = httpc->buf;
    p = memcpy(p, HEADER_HOST, STRLEN(HEADER_HOST)) + STRLEN(HEADER_HOST);
    p = memcpy(p, host, host_len) + host_len;
    p = memcpy(p, CRLF, STRLEN(CRLF)) + STRLEN(CRLF);

    assert(p - (void *)httpc->buf == httpc->headers_len);

    return httpc;
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
    const size_t value_len)
{
    switch (header)
    {
        case HTTP_HEADER_ACCEPT:
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
                    httpc->buf = realloc(httpc->buf, httpc->buf_len <<= 1);

                p = memcpy(p, HEADER_ACCEPT, STRLEN(HEADER_ACCEPT)) + STRLEN(HEADER_ACCEPT);
                p = memcpy(p, value, value_len) + value_len;
                p = memcpy(p, CRLF, STRLEN(CRLF)) + STRLEN(CRLF);

                assert(p - (void *)httpc->buf == httpc->headers_len);

                break;
            }
        }
    }
}

void uo_httpc_get(
    uo_httpc *httpc,
    const char *path,
    const size_t path_len,
    void *(*handle_response)(uo_http_res *, void *state),
    void *state)
{
    void *request = httpc->buf + httpc->headers_len;

    void *p = request;
    p = memcpy(p, HTTP_GET, STRLEN(HTTP_GET)) + STRLEN(HTTP_GET);
    p = memcpy(p, path, path_len) + path_len;
    p = memcpy(p, HTTP_1_1, STRLEN(HTTP_1_1)) + STRLEN(HTTP_1_1);
    p = memcpy(p, CRLF, STRLEN(CRLF)) + STRLEN(CRLF);
    p = memcpy(p, httpc->buf, httpc->headers_len) + httpc->headers_len;
    p = memcpy(p, CRLF, STRLEN(CRLF)) + STRLEN(CRLF);
    httpc->request_len = p - request;

    uo_cb *cb = uo_cb_create(uo_cb_opt_invoke_once);

    uo_cb_append(cb, (void *(*)(void *, void *))uo_httpc_make_request);
    uo_cb_append(cb, (void *(*)(void *, void *))handle_response);

    uo_cb_invoke_async(cb, httpc, state, NULL);
}
