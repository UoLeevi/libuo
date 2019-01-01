#include "uo_http_method.h"
#include "uo_mem.h"

#include <string.h>

UO_HTTP_METHOD uo_http_method_parse(
    uo_buf buf)
{
    char *cr = memchr(buf, '\r', uo_buf_get_len_before_ptr(buf));

    if (uo_mem_cmp_str_literal(cr - UO_STRLEN(" HTTP/1.1"), " HTTP/1.1") != 0)
        return UO_HTTP_METHOD_INVALID;

    switch (buf[0])
    {
        case 'G': return uo_mem_cmp_str_literal(buf, "GET ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_GET;

        case 'H': return uo_mem_cmp_str_literal(buf, "HEAD ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_HEAD;

        case 'D': return uo_mem_cmp_str_literal(buf, "DELETE ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_DELETE;

        case 'O': return uo_mem_cmp_str_literal(buf, "OPTIONS ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_OPTIONS;
        
        case 'T': return uo_mem_cmp_str_literal(buf, "TRACE ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_TRACE;

        case 'C': return uo_mem_cmp_str_literal(buf, "CONNECT ")
            ? UO_HTTP_METHOD_INVALID
            : UO_HTTP_1_1_METHOD_CONNECT;

        case 'P': return uo_mem_cmp_str_literal(buf, "POST ")
            ? uo_mem_cmp_str_literal(buf, "PUT ")
                ? UO_HTTP_METHOD_INVALID
                : UO_HTTP_1_1_METHOD_PUT
            : UO_HTTP_1_1_METHOD_POST;

        default: return UO_HTTP_METHOD_INVALID;
    }
}
