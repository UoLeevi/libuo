#ifndef UO_HTTP_STATUS_H
#define UO_HTTP_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#define UO_HTTP_STATUS_100 "100 Continue"
#define UO_HTTP_STATUS_101 "101 Switching Protocols"
#define UO_HTTP_STATUS_200 "200 OK"
#define UO_HTTP_STATUS_201 "201 Created"
#define UO_HTTP_STATUS_202 "202 Accepted"
#define UO_HTTP_STATUS_203 "203 Non-Authoritative Information"
#define UO_HTTP_STATUS_204 "204 No Content"
#define UO_HTTP_STATUS_205 "205 Reset Content"
#define UO_HTTP_STATUS_206 "206 Partial Content"
#define UO_HTTP_STATUS_300 "300 Multiple Choices"
#define UO_HTTP_STATUS_301 "301 Moved Permanently"
#define UO_HTTP_STATUS_302 "302 Found"
#define UO_HTTP_STATUS_303 "303 See Other"
#define UO_HTTP_STATUS_304 "304 Not Modified"
#define UO_HTTP_STATUS_305 "305 Use Proxy"
#define UO_HTTP_STATUS_307 "307 Temporary Redirect"
#define UO_HTTP_STATUS_400 "400 Bad Request"
#define UO_HTTP_STATUS_401 "401 Unauthorized"
#define UO_HTTP_STATUS_402 "402 Payment Required"
#define UO_HTTP_STATUS_403 "403 Forbidden"
#define UO_HTTP_STATUS_404 "404 Not Found"
#define UO_HTTP_STATUS_405 "405 Method Not Allowed"
#define UO_HTTP_STATUS_406 "406 Not Acceptable"
#define UO_HTTP_STATUS_407 "407 Proxy Authentication Required"
#define UO_HTTP_STATUS_408 "408 Request Time-out"
#define UO_HTTP_STATUS_409 "409 Conflict"
#define UO_HTTP_STATUS_410 "410 Gone"
#define UO_HTTP_STATUS_411 "411 Length Required"
#define UO_HTTP_STATUS_412 "412 Precondition Failed"
#define UO_HTTP_STATUS_413 "413 Request Entity Too Large"
#define UO_HTTP_STATUS_414 "414 Request-URI Too Large"
#define UO_HTTP_STATUS_415 "415 Unsupported Media Type"
#define UO_HTTP_STATUS_416 "416 Requested range not satisfied"
#define UO_HTTP_STATUS_417 "417 Expectation Failed"
#define UO_HTTP_STATUS_500 "500 Internal Server Error"
#define UO_HTTP_STATUS_501 "501 Not Implemented"
#define UO_HTTP_STATUS_502 "502 Bad Gateway"
#define UO_HTTP_STATUS_503 "503 Service Unavailable"
#define UO_HTTP_STATUS_504 "504 Gateway Time-out"
#define UO_HTTP_STATUS_505 "505 HTTP Version not supported"

typedef enum uo_http_status
{
    UO_HTTP_STATUS_INVALID,
    UO_HTTP_100, // Continue
    UO_HTTP_101, // Switching Protocols
    UO_HTTP_200, // OK
    UO_HTTP_201, // Created
    UO_HTTP_202, // Accepted
    UO_HTTP_203, // Non-Authoritative Information
    UO_HTTP_204, // No Content
    UO_HTTP_205, // Reset Content
    UO_HTTP_206, // Partial Content
    UO_HTTP_300, // Multiple Choices
    UO_HTTP_301, // Moved Permanently
    UO_HTTP_302, // Found
    UO_HTTP_303, // See Other
    UO_HTTP_304, // Not Modified
    UO_HTTP_305, // Use Proxy
    UO_HTTP_307, // Temporary Redirect
    UO_HTTP_400, // Bad Request
    UO_HTTP_401, // Unauthorized
    UO_HTTP_402, // Payment Required
    UO_HTTP_403, // Forbidden
    UO_HTTP_404, // Not Found
    UO_HTTP_405, // Method Not Allowed
    UO_HTTP_406, // Not Acceptable
    UO_HTTP_407, // Proxy Authentication Required
    UO_HTTP_408, // Request Time-out
    UO_HTTP_409, // Conflict
    UO_HTTP_410, // Gone
    UO_HTTP_411, // Length Required
    UO_HTTP_412, // Precondition Failed
    UO_HTTP_413, // Request Entity Too Large
    UO_HTTP_414, // Request-URI Too Large
    UO_HTTP_415, // Unsupported Media Type
    UO_HTTP_416, // Requested range not satisfied
    UO_HTTP_417, // Expectation Failed
    UO_HTTP_500, // Internal Server Error
    UO_HTTP_501, // Not Implemented
    UO_HTTP_502, // Bad Gateway
    UO_HTTP_503, // Service Unavailable
    UO_HTTP_504, // Gateway Time-out
    UO_HTTP_505, // HTTP Version not supported
} uo_http_status;

#ifdef __cplusplus
}
#endif

#endif
