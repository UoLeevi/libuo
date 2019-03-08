#ifndef UO_HTTP_MSG_H
#define UO_HTTP_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_ver.h"
#include "uo_http_method.h"
#include "uo_http_status.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>

#define uo_http_req_set_header uo_http_msg_set_header
#define uo_http_res_set_header uo_http_msg_set_header

#define uo_http_req_set_content uo_http_msg_set_content
#define uo_http_res_set_content uo_http_msg_set_content

#define uo_http_req_get_body uo_http_msg_get_body
#define uo_http_res_get_body uo_http_msg_get_body

#define uo_http_req_get_header uo_http_msg_get_header
#define uo_http_res_get_header uo_http_msg_get_header

#define uo_http_req_get_version uo_http_msg_get_version
#define uo_http_res_get_version uo_http_msg_get_version

typedef struct uo_strhashtbl uo_strhashtbl;

/**
 * @brief type that can represents HTTP request or response
 * 
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
 */
typedef struct uo_http_msg
{
    uo_buf *buf;
    char *uri;
    ptrdiff_t start_line;
    size_t start_line_len;
    uo_strhashtbl *headers;
    ptrdiff_t body;
    size_t body_len;
    struct
    {
        bool is_invalid;
    } state;
    void *finstack;
} uo_http_msg, uo_http_req, uo_http_res;

uo_http_msg *uo_http_msg_create(
    uo_buf *buf);

bool uo_http_msg_set_header(
    uo_http_msg *,
    const char *header_name,
    const char *header_value);

bool uo_http_msg_set_content(
    uo_http_msg *,
    const char *content,
    const char *content_type,
    size_t content_len);

bool uo_http_req_set_request_line(
    uo_http_req *,
    uo_http_method,
    const char *target,
    uo_http_ver);

bool uo_http_res_set_status_line(
    uo_http_res *,
    uo_http_status,
    uo_http_ver);

bool uo_http_res_set_content_from_file(
    uo_http_res *,
    const char *filename);

bool uo_http_msg_parse_start_line(
    uo_http_msg *);

bool uo_http_msg_parse_headers(
    uo_http_msg *);

bool uo_http_msg_parse_body(
    uo_http_msg *);

char *uo_http_msg_get_body(
    uo_http_msg *);

char *uo_http_msg_get_header(
    uo_http_msg *,
    const char *header_name);

uo_http_ver uo_http_msg_get_version(
    uo_http_msg *);

uo_http_method uo_http_req_get_method(
    uo_http_req *);

char *uo_http_req_get_uri(
    uo_http_req *);

uo_http_status uo_http_res_get_status(
    uo_http_res *);

void uo_http_msg_write_to_buf(
    uo_http_msg *,
    uo_buf *dst);

void uo_http_msg_destroy(
    uo_http_msg *);

#ifdef __cplusplus
}
#endif

#endif
