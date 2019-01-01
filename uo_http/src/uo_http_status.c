#include "uo_http_status.h"
#include "uo_mem.h"

#define UO_HTTP_1_1_STATUS_100_LINE "HTTP/1.1 100 Continue" "\r\n"
#define UO_HTTP_1_1_STATUS_101_LINE "HTTP/1.1 101 Switching Protocols" "\r\n"
#define UO_HTTP_1_1_STATUS_200_LINE "HTTP/1.1 200 OK" "\r\n"
#define UO_HTTP_1_1_STATUS_201_LINE "HTTP/1.1 201 Created" "\r\n"
#define UO_HTTP_1_1_STATUS_202_LINE "HTTP/1.1 202 Accepted" "\r\n"
#define UO_HTTP_1_1_STATUS_203_LINE "HTTP/1.1 203 Non-Authoritative Information" "\r\n"
#define UO_HTTP_1_1_STATUS_204_LINE "HTTP/1.1 204 No Content" "\r\n"
#define UO_HTTP_1_1_STATUS_205_LINE "HTTP/1.1 205 Reset Content" "\r\n"
#define UO_HTTP_1_1_STATUS_206_LINE "HTTP/1.1 206 Partial Content" "\r\n"
#define UO_HTTP_1_1_STATUS_300_LINE "HTTP/1.1 300 Multiple Choices" "\r\n"
#define UO_HTTP_1_1_STATUS_301_LINE "HTTP/1.1 301 Moved Permanently" "\r\n"
#define UO_HTTP_1_1_STATUS_302_LINE "HTTP/1.1 302 Found" "\r\n"
#define UO_HTTP_1_1_STATUS_303_LINE "HTTP/1.1 303 See Other" "\r\n"
#define UO_HTTP_1_1_STATUS_304_LINE "HTTP/1.1 304 Not Modified" "\r\n"
#define UO_HTTP_1_1_STATUS_305_LINE "HTTP/1.1 305 Use Proxy" "\r\n"
#define UO_HTTP_1_1_STATUS_307_LINE "HTTP/1.1 307 Temporary Redirect" "\r\n"
#define UO_HTTP_1_1_STATUS_400_LINE "HTTP/1.1 400 Bad Request" "\r\n"
#define UO_HTTP_1_1_STATUS_401_LINE "HTTP/1.1 401 Unauthorized" "\r\n"
#define UO_HTTP_1_1_STATUS_402_LINE "HTTP/1.1 402 Payment Required" "\r\n"
#define UO_HTTP_1_1_STATUS_403_LINE "HTTP/1.1 403 Forbidden" "\r\n"
#define UO_HTTP_1_1_STATUS_404_LINE "HTTP/1.1 404 Not Found" "\r\n"
#define UO_HTTP_1_1_STATUS_405_LINE "HTTP/1.1 405 Method Not Allowed" "\r\n"
#define UO_HTTP_1_1_STATUS_406_LINE "HTTP/1.1 406 Not Acceptable" "\r\n"
#define UO_HTTP_1_1_STATUS_407_LINE "HTTP/1.1 407 Proxy Authentication Required" "\r\n"
#define UO_HTTP_1_1_STATUS_408_LINE "HTTP/1.1 408 Request Time-out" "\r\n"
#define UO_HTTP_1_1_STATUS_409_LINE "HTTP/1.1 409 Conflict" "\r\n"
#define UO_HTTP_1_1_STATUS_410_LINE "HTTP/1.1 410 Gone" "\r\n"
#define UO_HTTP_1_1_STATUS_411_LINE "HTTP/1.1 411 Length Required" "\r\n"
#define UO_HTTP_1_1_STATUS_412_LINE "HTTP/1.1 412 Precondition Failed" "\r\n"
#define UO_HTTP_1_1_STATUS_413_LINE "HTTP/1.1 413 Request Entity Too Large" "\r\n"
#define UO_HTTP_1_1_STATUS_414_LINE "HTTP/1.1 414 Request-URI Too Large" "\r\n"
#define UO_HTTP_1_1_STATUS_415_LINE "HTTP/1.1 415 Unsupported Media Type" "\r\n"
#define UO_HTTP_1_1_STATUS_416_LINE "HTTP/1.1 416 Requested range not satisfied" "\r\n"
#define UO_HTTP_1_1_STATUS_417_LINE "HTTP/1.1 417 Expectation Failed" "\r\n"
#define UO_HTTP_1_1_STATUS_500_LINE "HTTP/1.1 500 Internal Server Error" "\r\n"
#define UO_HTTP_1_1_STATUS_501_LINE "HTTP/1.1 501 Not Implemented" "\r\n"
#define UO_HTTP_1_1_STATUS_502_LINE "HTTP/1.1 502 Bad Gateway" "\r\n"
#define UO_HTTP_1_1_STATUS_503_LINE "HTTP/1.1 503 Service Unavailable" "\r\n"
#define UO_HTTP_1_1_STATUS_504_LINE "HTTP/1.1 504 Gateway Time-out" "\r\n"
#define UO_HTTP_1_1_STATUS_505_LINE "HTTP/1.1 505 HTTP Version not supported" "\r\n"

#include <string.h>

bool uo_http_status_append_line(
    uo_buf buf,
    UO_HTTP_STATUS status)
{
    unsigned char *p = uo_buf_get_ptr(buf);

    switch (status)
    {
        case UO_HTTP_1_1_STATUS_100: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_100_LINE); break;
        case UO_HTTP_1_1_STATUS_101: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_101_LINE); break;
        case UO_HTTP_1_1_STATUS_200: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_200_LINE); break;
        case UO_HTTP_1_1_STATUS_201: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_201_LINE); break;
        case UO_HTTP_1_1_STATUS_202: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_202_LINE); break;
        case UO_HTTP_1_1_STATUS_203: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_203_LINE); break;
        case UO_HTTP_1_1_STATUS_204: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_204_LINE); break;
        case UO_HTTP_1_1_STATUS_205: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_205_LINE); break;
        case UO_HTTP_1_1_STATUS_206: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_206_LINE); break;
        case UO_HTTP_1_1_STATUS_300: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_300_LINE); break;
        case UO_HTTP_1_1_STATUS_301: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_301_LINE); break;
        case UO_HTTP_1_1_STATUS_302: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_302_LINE); break;
        case UO_HTTP_1_1_STATUS_303: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_303_LINE); break;
        case UO_HTTP_1_1_STATUS_304: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_304_LINE); break;
        case UO_HTTP_1_1_STATUS_305: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_305_LINE); break;
        case UO_HTTP_1_1_STATUS_307: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_307_LINE); break;
        case UO_HTTP_1_1_STATUS_400: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_400_LINE); break;
        case UO_HTTP_1_1_STATUS_401: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_401_LINE); break;
        case UO_HTTP_1_1_STATUS_402: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_402_LINE); break;
        case UO_HTTP_1_1_STATUS_403: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_403_LINE); break;
        case UO_HTTP_1_1_STATUS_404: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_404_LINE); break;
        case UO_HTTP_1_1_STATUS_405: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_405_LINE); break;
        case UO_HTTP_1_1_STATUS_406: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_406_LINE); break;
        case UO_HTTP_1_1_STATUS_407: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_407_LINE); break;
        case UO_HTTP_1_1_STATUS_408: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_408_LINE); break;
        case UO_HTTP_1_1_STATUS_409: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_409_LINE); break;
        case UO_HTTP_1_1_STATUS_410: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_410_LINE); break;
        case UO_HTTP_1_1_STATUS_411: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_411_LINE); break;
        case UO_HTTP_1_1_STATUS_412: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_412_LINE); break;
        case UO_HTTP_1_1_STATUS_413: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_413_LINE); break;
        case UO_HTTP_1_1_STATUS_414: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_414_LINE); break;
        case UO_HTTP_1_1_STATUS_415: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_415_LINE); break;
        case UO_HTTP_1_1_STATUS_416: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_416_LINE); break;
        case UO_HTTP_1_1_STATUS_417: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_417_LINE); break;
        case UO_HTTP_1_1_STATUS_500: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_500_LINE); break;
        case UO_HTTP_1_1_STATUS_501: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_501_LINE); break;
        case UO_HTTP_1_1_STATUS_502: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_502_LINE); break;
        case UO_HTTP_1_1_STATUS_503: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_503_LINE); break;
        case UO_HTTP_1_1_STATUS_504: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_504_LINE); break;
        case UO_HTTP_1_1_STATUS_505: p = uo_mem_append_str_literal(p, UO_HTTP_1_1_STATUS_505_LINE); break;
        default: return false;
    }

    uo_buf_set_ptr_abs(buf, p - buf);

    return true;
}

size_t uo_http_status_get_line_len(
    UO_HTTP_STATUS status)
{
    switch (status)
    {
        case UO_HTTP_1_1_STATUS_100: return UO_STRLEN(UO_HTTP_1_1_STATUS_100_LINE);
        case UO_HTTP_1_1_STATUS_101: return UO_STRLEN(UO_HTTP_1_1_STATUS_101_LINE);
        case UO_HTTP_1_1_STATUS_200: return UO_STRLEN(UO_HTTP_1_1_STATUS_200_LINE);
        case UO_HTTP_1_1_STATUS_201: return UO_STRLEN(UO_HTTP_1_1_STATUS_201_LINE);
        case UO_HTTP_1_1_STATUS_202: return UO_STRLEN(UO_HTTP_1_1_STATUS_202_LINE);
        case UO_HTTP_1_1_STATUS_203: return UO_STRLEN(UO_HTTP_1_1_STATUS_203_LINE);
        case UO_HTTP_1_1_STATUS_204: return UO_STRLEN(UO_HTTP_1_1_STATUS_204_LINE);
        case UO_HTTP_1_1_STATUS_205: return UO_STRLEN(UO_HTTP_1_1_STATUS_205_LINE);
        case UO_HTTP_1_1_STATUS_206: return UO_STRLEN(UO_HTTP_1_1_STATUS_206_LINE);
        case UO_HTTP_1_1_STATUS_300: return UO_STRLEN(UO_HTTP_1_1_STATUS_300_LINE);
        case UO_HTTP_1_1_STATUS_301: return UO_STRLEN(UO_HTTP_1_1_STATUS_301_LINE);
        case UO_HTTP_1_1_STATUS_302: return UO_STRLEN(UO_HTTP_1_1_STATUS_302_LINE);
        case UO_HTTP_1_1_STATUS_303: return UO_STRLEN(UO_HTTP_1_1_STATUS_303_LINE);
        case UO_HTTP_1_1_STATUS_304: return UO_STRLEN(UO_HTTP_1_1_STATUS_304_LINE);
        case UO_HTTP_1_1_STATUS_305: return UO_STRLEN(UO_HTTP_1_1_STATUS_305_LINE);
        case UO_HTTP_1_1_STATUS_307: return UO_STRLEN(UO_HTTP_1_1_STATUS_307_LINE);
        case UO_HTTP_1_1_STATUS_400: return UO_STRLEN(UO_HTTP_1_1_STATUS_400_LINE);
        case UO_HTTP_1_1_STATUS_401: return UO_STRLEN(UO_HTTP_1_1_STATUS_401_LINE);
        case UO_HTTP_1_1_STATUS_402: return UO_STRLEN(UO_HTTP_1_1_STATUS_402_LINE);
        case UO_HTTP_1_1_STATUS_403: return UO_STRLEN(UO_HTTP_1_1_STATUS_403_LINE);
        case UO_HTTP_1_1_STATUS_404: return UO_STRLEN(UO_HTTP_1_1_STATUS_404_LINE);
        case UO_HTTP_1_1_STATUS_405: return UO_STRLEN(UO_HTTP_1_1_STATUS_405_LINE);
        case UO_HTTP_1_1_STATUS_406: return UO_STRLEN(UO_HTTP_1_1_STATUS_406_LINE);
        case UO_HTTP_1_1_STATUS_407: return UO_STRLEN(UO_HTTP_1_1_STATUS_407_LINE);
        case UO_HTTP_1_1_STATUS_408: return UO_STRLEN(UO_HTTP_1_1_STATUS_408_LINE);
        case UO_HTTP_1_1_STATUS_409: return UO_STRLEN(UO_HTTP_1_1_STATUS_409_LINE);
        case UO_HTTP_1_1_STATUS_410: return UO_STRLEN(UO_HTTP_1_1_STATUS_410_LINE);
        case UO_HTTP_1_1_STATUS_411: return UO_STRLEN(UO_HTTP_1_1_STATUS_411_LINE);
        case UO_HTTP_1_1_STATUS_412: return UO_STRLEN(UO_HTTP_1_1_STATUS_412_LINE);
        case UO_HTTP_1_1_STATUS_413: return UO_STRLEN(UO_HTTP_1_1_STATUS_413_LINE);
        case UO_HTTP_1_1_STATUS_414: return UO_STRLEN(UO_HTTP_1_1_STATUS_414_LINE);
        case UO_HTTP_1_1_STATUS_415: return UO_STRLEN(UO_HTTP_1_1_STATUS_415_LINE);
        case UO_HTTP_1_1_STATUS_416: return UO_STRLEN(UO_HTTP_1_1_STATUS_416_LINE);
        case UO_HTTP_1_1_STATUS_417: return UO_STRLEN(UO_HTTP_1_1_STATUS_417_LINE);
        case UO_HTTP_1_1_STATUS_500: return UO_STRLEN(UO_HTTP_1_1_STATUS_500_LINE);
        case UO_HTTP_1_1_STATUS_501: return UO_STRLEN(UO_HTTP_1_1_STATUS_501_LINE);
        case UO_HTTP_1_1_STATUS_502: return UO_STRLEN(UO_HTTP_1_1_STATUS_502_LINE);
        case UO_HTTP_1_1_STATUS_503: return UO_STRLEN(UO_HTTP_1_1_STATUS_503_LINE);
        case UO_HTTP_1_1_STATUS_504: return UO_STRLEN(UO_HTTP_1_1_STATUS_504_LINE);
        case UO_HTTP_1_1_STATUS_505: return UO_STRLEN(UO_HTTP_1_1_STATUS_505_LINE);
        default: return 0;
    }
}
