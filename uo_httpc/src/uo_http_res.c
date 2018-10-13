#include "uo_http_res.h"

#include <stdlib.h>
#include <string.h>

uo_http_res *uo_http_res_create(
    const char *headers, 
    const size_t headers_len, 
    const char *body, 
    const size_t body_len)
{
    uo_http_res *http_res = malloc(sizeof *http_res);

    http_res->headers = calloc(headers_len + body_len + 2, sizeof *http_res->headers);
    http_res->body = http_res->headers + headers_len + 1;

    memcpy(http_res->headers, headers, headers_len);
    memcpy(http_res->body, body, body_len);

    http_res->headers_len = headers_len;
    http_res->body_len = body_len;

    return http_res;
}

void uo_http_res_destroy(
    uo_http_res *http_res)
{
    free(http_res->headers);
    free(http_res);
}
