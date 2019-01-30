#ifndef UO_HTTP_VER_H
#define UO_HTTP_VER_H

#ifdef __cplusplus
extern "C" {
#endif

#define UO_HTTP_VER_1_0 "HTTP/1.0"
#define UO_HTTP_VER_1_1 "HTTP/1.1"
#define UO_HTTP_VER_2   "HTTP/2"

typedef enum uo_http_ver
{
    UO_HTTP_VER_INVALID,
    UO_HTTP_1_0, // HTTP/1.0
    UO_HTTP_1_1, // HTTP/1.1
    UO_HTTP_2    // HTTP/2
} uo_http_ver;

#ifdef __cplusplus
}
#endif

#endif
