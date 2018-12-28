#ifndef UO_HTTP_REQUEST_H
#define UO_HTTP_REQUEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_buf.h"

#include <stdbool.h>

typedef union uo_http_request
{
    uo_buf buf;
    struct
    {
        char *method;
        char *target;
        char *version;
    } start_line;
    char *path;
} uo_http_request;

uo_http_request *uo_http_request_create(void);

bool uo_http_request_parse_start_line(
    uo_http_request *,
    uo_buf);

bool uo_http_request_parse_path(
    uo_http_request *,
    const char *root_dir_path);

void uo_http_request_destroy(
    uo_http_request *);

#ifdef __cplusplus
}
#endif

#endif
