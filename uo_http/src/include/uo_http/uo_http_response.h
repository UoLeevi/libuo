#ifndef UO_HTTP_RESPONSE_H
#define UO_HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_status.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>

// uo_http_response field 'body' is offset from buffer start
// storing pointers would be proplematic if buffer is reallocated

typedef struct uo_http_response
{
    uo_buf *buf;
    UO_HTTP_STATUS status;
    void *headers;
    ptrdiff_t body;
    size_t content_len;
    struct
    {
        bool is_fully_parsed;
        bool is_recv_headers_evt_raised;
    } state;
} uo_http_response;

uo_http_response *uo_http_response_create(
    uo_buf *buf);

bool uo_http_response_set_status(
    uo_http_response *,
    UO_HTTP_STATUS);

bool uo_http_response_set_header(
    uo_http_response *,
    const char *header_name,
    char *header_value);

bool uo_http_response_set_content(
    uo_http_response *,
    const char *content,
    char *content_type,
    size_t content_len);

bool uo_http_response_set_file(
    uo_http_response *,
    const char *filename);

bool uo_http_response_parse_status_line(
    uo_http_response *);

bool uo_http_response_parse_headers(
    uo_http_response *);

bool uo_http_response_parse_body(
    uo_http_response *);

char *uo_http_response_get_body(
    uo_http_response *);

void uo_http_response_destroy(
    uo_http_response *);

#ifdef __cplusplus
}
#endif

#endif
