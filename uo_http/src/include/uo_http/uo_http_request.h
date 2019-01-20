#ifndef UO_HTTP_REQUEST_H
#define UO_HTTP_REQUEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_method.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>

// uo_http_request fields 'target' and 'body' are offsets from buffer start
// storing pointers would be proplematic if buffer is reallocated

typedef struct uo_http_request
{
    uo_buf *buf;
    UO_HTTP_METHOD method;
    ptrdiff_t target;
    void *headers;
    ptrdiff_t body;
    size_t content_len;
    struct
    {
        bool is_fully_parsed;
        bool is_recv_headers_evt_raised;
    } state;
} uo_http_request;

uo_http_request *uo_http_request_create(
    uo_buf *buf);

bool uo_http_request_set_method(
    uo_http_request *,
    UO_HTTP_METHOD);

bool uo_http_request_set_target(
    uo_http_request *,
    const char *target);

bool uo_http_request_set_header(
    uo_http_request *,
    const char *header_name,
    char *header_value);

bool uo_http_request_set_content(
    uo_http_request *,
    const char *content,
    char *content_type,
    size_t content_len);

bool uo_http_request_parse_start_line(
    uo_http_request *);

bool uo_http_request_parse_headers(
    uo_http_request *);

bool uo_http_request_parse_body(
    uo_http_request *);

char *uo_http_request_get_target(
    uo_http_request *);

char *uo_http_request_get_body(
    uo_http_request *);

void uo_http_request_destroy(
    uo_http_request *);

#ifdef __cplusplus
}
#endif

#endif
