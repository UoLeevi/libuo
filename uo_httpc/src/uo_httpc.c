#include "uo_httpc.h"
#include "uo_cb.h"
#include "uo_sock.h"

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

#define BUF_GROW 0x10000

#define STRLEN(s) (sizeof(s) / sizeof(s[0]) - 1)

#define CRLF "\r\n"

#define HTTP_GET "GET "
#define HTTP_1_1 " HTTP/1.1"

#define HEADER_HOST "Host: "
#define HEADER_ACCEPT "Accept: "
#define HEADER_CONTENT_LENGTH "Content-Length: "

static uintmax_t read_content_length(
    char *src)
{
    char *headers_end = strstr(src, CRLF CRLF);
    if (!headers_end)
        return 0;

    size_t headers_len = headers_end - src;
    char headers[headers_len + 1];
    headers[headers_len] = '\0';
    memcpy(headers, src, headers_len);

    for (int i = 0; headers[i]; ++i)
        headers[i] = tolower(headers[i]);

    char *content_length_header = strstr(headers, "content-length: ");
    if (!content_length_header)
        return 0;

    char *endptr;

    return strtoumax(content_length_header + 16, &endptr, 10);
}

uo_http_res *uo_http_res_create(
    const char *headers, 
    const size_t headers_len, 
    const char *body, 
    const size_t body_len);

static uo_http_res *uo_httpc_make_request(
    uo_httpc *httpc,
    void *_)
{
    int sockfd = socket(httpc->serv_addrinfo->ai_family, SOCK_STREAM, IPPROTO_TCP);

    if (connect(sockfd, httpc->serv_addrinfo->ai_addr, httpc->serv_addrinfo->ai_addrlen) == -1)
        return NULL;

    char *request = httpc->buf + httpc->headers_len;

    if (send(sockfd, request, httpc->request_len, 0) == -1)
        return NULL;

    char *response = request + httpc->request_len;
    size_t response_buf_len = httpc->buf_len - (response - httpc->buf);

    uintmax_t content_length = 0;
    char *p = response;
    ssize_t len;

    while (!content_length)
    {
        p += len = recv(sockfd, p, response_buf_len, 0);
        if (len == -1)
            return NULL;
        response_buf_len = httpc->buf_len - (p - httpc->buf);
        if (!response_buf_len) 
        {
            ptrdiff_t pdiff = p - httpc->buf;
            ptrdiff_t responsediff = response - httpc->buf;
            httpc->buf = realloc(httpc->buf, httpc->buf_len += BUF_GROW);
            p = httpc->buf + pdiff;
            response = httpc->buf + responsediff;
        }
            
        *p = '\0';
        content_length = read_content_length(response);
    }

    char *headers_end = strstr(response, CRLF CRLF);
    assert(headers_end);
    headers_end += STRLEN(CRLF CRLF);

    while (p - headers_end < content_length)
    {
        p += len = recv(sockfd, p, response_buf_len, 0);
        if (len == -1)
            return NULL;
        response_buf_len = httpc->buf_len - (p - httpc->buf);
        if (!response_buf_len) 
        {
            ptrdiff_t pdiff = p - httpc->buf;
            ptrdiff_t responsediff = response - httpc->buf;
            ptrdiff_t headers_enddiff = headers_end - httpc->buf;
            httpc->buf = realloc(httpc->buf, httpc->buf_len += BUF_GROW);
            p = httpc->buf + pdiff;
            response = httpc->buf + responsediff;
            headers_end = httpc->buf + headers_enddiff;
        }
    }

    httpc->response_len = p - response;

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return uo_http_res_create(
        response,
        headers_end - 2 - response,
        headers_end,
        p - headers_end);
}

void uo_httpc_init(
    size_t thrd_count)
{
    uo_cb_init(thrd_count);
    uo_sock_init();
}

uo_httpc *uo_httpc_create(
    const char *host, 
    const size_t host_len)
{
    uo_httpc *httpc = malloc(sizeof(uo_httpc));

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    int s = getaddrinfo(host, "80", &hints, &httpc->serv_addrinfo);
    
    if (s != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(s));
        free(httpc);
        return NULL;
    }

    httpc->buf = malloc(BUF_GROW);
    httpc->buf_len = BUF_GROW;
    httpc->header_flags = HTTP_HEADER_HOST;
    httpc->headers_len = STRLEN(HEADER_HOST) + host_len + STRLEN(CRLF);

    while (httpc->buf_len < httpc->headers_len)
        httpc->buf = realloc(httpc->buf, httpc->buf_len += BUF_GROW);

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
                    httpc->buf = realloc(httpc->buf, httpc->buf_len += BUF_GROW);

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
