#ifndef UO_HTTPC_H
#define UO_HTTPC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_http_res.h"
#include "uo_sock.h"

#include <stddef.h>
#include <stdbool.h>

    typedef enum HTTP_HEADER_FLAGS
    {
        HTTP_HEADER_NONE = 0,
        HTTP_HEADER_HOST = 1,
        HTTP_HEADER_ACCEPT = 2,
    } HTTP_HEADER_FLAGS;

    typedef enum UO_HTTPC_OPT {
	    UO_HTTPC_OPT_TLS = 1 << 0
    } UO_HTTPC_OPT;

    typedef struct uo_httpc
    {
        UO_HTTPC_OPT opt;
        HTTP_HEADER_FLAGS header_flags;
        struct addrinfo *serv_addrinfo;
        size_t headers_len;
        size_t request_len;
        size_t buf_len;
        char *buf;
    } uo_httpc;

    bool uo_httpc_init(
        size_t thrd_count);

    uo_httpc *uo_httpc_create(
        const char *host, 
        const size_t host_len,
        UO_HTTPC_OPT opt);
        
    void uo_httpc_destroy(
        uo_httpc *httpc);

    void uo_httpc_set_header(
        uo_httpc *httpc,
        HTTP_HEADER_FLAGS header,
        const char *value,
        const size_t value_len);

    void uo_httpc_get(
        uo_httpc *httpc,
        const char *path,
        const size_t path_len,
        void *(*handle_response)(uo_http_res *, void *state),
        void *state);

#ifdef __cplusplus
}
#endif

#endif