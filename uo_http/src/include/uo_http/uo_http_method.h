#ifndef UO_HTTP_METHOD_H
#define UO_HTTP_METHOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_buf.h"

typedef enum UO_HTTP_METHOD
{
    UO_HTTP_METHOD_INVALID,
    UO_HTTP_1_1_METHOD_GET,
    UO_HTTP_1_1_METHOD_HEAD,
    UO_HTTP_1_1_METHOD_PUT,
    UO_HTTP_1_1_METHOD_DELETE,
    UO_HTTP_1_1_METHOD_OPTIONS,
    UO_HTTP_1_1_METHOD_TRACE,
    UO_HTTP_1_1_METHOD_POST,
    UO_HTTP_1_1_METHOD_CONNECT
} UO_HTTP_METHOD;

UO_HTTP_METHOD uo_http_method_parse(
    uo_buf);

#ifdef __cplusplus
}
#endif

#endif
