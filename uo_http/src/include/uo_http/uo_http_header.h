#ifndef UO_HTTP_HEADER_H
#define UO_HTTP_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#define UO_HTTP_HEADER_CONNECTION       "Connection: "
#define UO_HTTP_HEADER_SERVER           "Server: "
#define UO_HTTP_HEADER_CONTENT_LENGTH   "Content-Length: "
#define UO_HTTP_HEADER_CONTENT_TYPE     "Content-Type: "

#define UO_HTTP_HEADER_CONTENT_TYPE_HTML    UO_HTTP_HEADER_CONTENT_TYPE "text/html; charset=utf-8" "\r\n"
#define UO_HTTP_HEADER_CONTENT_TYPE_CSS     UO_HTTP_HEADER_CONTENT_TYPE "text/css; charset=utf-8" "\r\n"
#define UO_HTTP_HEADER_CONTENT_TYPE_JS      UO_HTTP_HEADER_CONTENT_TYPE "application/javascript; charset=utf-8" "\r\n"

const char *uo_http_header_content_type_for_path(
    const char *path);

#ifdef __cplusplus
}
#endif

#endif
