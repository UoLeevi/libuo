#ifndef UO_HTTP_METHOD_H
#define UO_HTTP_METHOD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum uo_http_method
{
    UO_HTTP_METHOD_INVALID,
    UO_HTTP_GET,
    UO_HTTP_HEAD,
    UO_HTTP_PUT,
    UO_HTTP_DELETE,
    UO_HTTP_OPTIONS,
    UO_HTTP_TRACE,
    UO_HTTP_POST,
    UO_HTTP_CONNECT
} uo_http_method;

#ifdef __cplusplus
}
#endif

#endif
