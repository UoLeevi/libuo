#ifndef UO_HTTP_MSG_H
#define UO_HTTP_MSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_ver.h"
#include "uo_http_method.h"
#include "uo_http_status.h"
#include "uo_hashtbl.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>

#define uo_http_req_set_header uo_http_msg_set_header
#define uo_http_res_set_header uo_http_msg_set_header

#define uo_http_req_set_content uo_http_msg_set_content
#define uo_http_res_set_content uo_http_msg_set_content

typedef struct uo_strhashtbl uo_strhashtbl;

/**
 * @brief type that can represents HTTP request or response
 * 
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
 */

typedef enum uo_http_msg_type
{
    UO_HTTP_MSG_TYPE_REQUEST,
    UO_HTTP_MSG_TYPE_RESPONSE
} uo_http_msg_type;

typedef enum uo_http_msg_role
{
    UO_HTTP_MSG_ROLE_RECV,
    UO_HTTP_MSG_ROLE_SEND
} uo_http_msg_role;

typedef struct uo_http_msg
{
    uo_strhashtbl headers;
    uo_buf *buf;
    char *body;
    size_t body_len;
    void *finstack;
    union
    {
        struct // only for HTTP request
        {
            char *method_sp_uri;    // Method SP Request-URI
            char *uri;
            char *version_crlf;
            uo_http_method method;
        };
        struct // only for HTTP response
        {
            char *version;
            char *status_sp_reason; // Status-Code SP Reason-Phrase
            uo_http_status status;
        };
    };
    uo_http_ver ver;
    union // used to store tempory state
    {
        ptrdiff_t parsing_offset;
        int req_handler_counter;
    } temp;
    struct
    {
        unsigned int is_invalid : 1;
        unsigned int type       : 1;
        unsigned int role       : 1;
        unsigned int start_line : 1;
        unsigned int headers    : 1;
        unsigned int body       : 1;
    } flags;
} uo_http_msg, uo_http_req, uo_http_res;

void uo_http_msg_create_at(
    uo_http_msg *,
    uo_buf *buf,
    uo_http_msg_type,
    uo_http_msg_role);

void uo_http_msg_destroy_at(
    uo_http_msg *);

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
    const char *uri,
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

char *uo_http_msg_get_header(
    uo_http_msg *,
    const char *header_name);

void uo_http_msg_write_to_buf(
    uo_http_msg *,
    uo_buf *dst);

#ifdef __cplusplus
}
#endif

#endif
