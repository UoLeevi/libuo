#ifndef UO_HTTP_RESPONSE_H
#define UO_HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_status.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_http_response
{
    uo_buf buf;
    UO_HTTP_STATUS status;
    void *headers;
    size_t content_len;
} uo_http_response;

uo_http_response *uo_http_response_create(void);

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

void uo_http_response_destroy(
    uo_http_response *);

#ifdef __cplusplus
}
#endif

#endif
