#ifndef UO_HTTP_STATUS_H
#define UO_HTTP_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_buf.h"

#include <stdbool.h>

typedef enum UO_HTTP_STATUS
{
    UO_HTTP_STATUS_INVALID,
    UO_HTTP_1_1_STATUS_100, // Continue
    UO_HTTP_1_1_STATUS_101, // Switching Protocols
    UO_HTTP_1_1_STATUS_200, // OK
    UO_HTTP_1_1_STATUS_201, // Created
    UO_HTTP_1_1_STATUS_202, // Accepted
    UO_HTTP_1_1_STATUS_203, // Non-Authoritative Information
    UO_HTTP_1_1_STATUS_204, // No Content
    UO_HTTP_1_1_STATUS_205, // Reset Content
    UO_HTTP_1_1_STATUS_206, // Partial Content
    UO_HTTP_1_1_STATUS_300, // Multiple Choices
    UO_HTTP_1_1_STATUS_301, // Moved Permanently
    UO_HTTP_1_1_STATUS_302, // Found
    UO_HTTP_1_1_STATUS_303, // See Other
    UO_HTTP_1_1_STATUS_304, // Not Modified
    UO_HTTP_1_1_STATUS_305, // Use Proxy
    UO_HTTP_1_1_STATUS_307, // Temporary Redirect
    UO_HTTP_1_1_STATUS_400, // Bad Request
    UO_HTTP_1_1_STATUS_401, // Unauthorized
    UO_HTTP_1_1_STATUS_402, // Payment Required
    UO_HTTP_1_1_STATUS_403, // Forbidden
    UO_HTTP_1_1_STATUS_404, // Not Found
    UO_HTTP_1_1_STATUS_405, // Method Not Allowed
    UO_HTTP_1_1_STATUS_406, // Not Acceptable
    UO_HTTP_1_1_STATUS_407, // Proxy Authentication Required
    UO_HTTP_1_1_STATUS_408, // Request Time-out
    UO_HTTP_1_1_STATUS_409, // Conflict
    UO_HTTP_1_1_STATUS_410, // Gone
    UO_HTTP_1_1_STATUS_411, // Length Required
    UO_HTTP_1_1_STATUS_412, // Precondition Failed
    UO_HTTP_1_1_STATUS_413, // Request Entity Too Large
    UO_HTTP_1_1_STATUS_414, // Request-URI Too Large
    UO_HTTP_1_1_STATUS_415, // Unsupported Media Type
    UO_HTTP_1_1_STATUS_416, // Requested range not satisfied
    UO_HTTP_1_1_STATUS_417, // Expectation Failed
    UO_HTTP_1_1_STATUS_500, // Internal Server Error
    UO_HTTP_1_1_STATUS_501, // Not Implemented
    UO_HTTP_1_1_STATUS_502, // Bad Gateway
    UO_HTTP_1_1_STATUS_503, // Service Unavailable
    UO_HTTP_1_1_STATUS_504, // Gateway Time-out
    UO_HTTP_1_1_STATUS_505, // HTTP Version not supported
} UO_HTTP_STATUS;

bool uo_http_status_append_line(
    uo_buf,
    UO_HTTP_STATUS);

size_t uo_http_status_get_line_len(
    UO_HTTP_STATUS);

#ifdef __cplusplus
}
#endif

#endif
