#ifndef UO_HTTP_RES_H
#define UO_HTTP_RES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

typedef struct uo_http_res
{   
    size_t body_len;
    size_t headers_len;
    char *body;
    char *headers;
} uo_http_res;

void uo_http_res_destroy(
    uo_http_res *http_res);

#ifdef __cplusplus
}
#endif

#endif