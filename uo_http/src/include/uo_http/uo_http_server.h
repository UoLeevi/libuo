#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef void (*uo_http_evt_handler)(uo_http_conn *, uo_cb *);

typedef struct uo_http_server
{
    struct
    {
        uo_http_evt_handler after_accept_handler;
        uo_http_evt_handler before_recv_handler;
        uo_http_evt_handler after_recv_handler;
        uo_http_evt_handler before_send_handler;
        uo_http_evt_handler after_send_handler;
        uo_http_evt_handler after_close_handler;
    } evt;
    struct
    {
        bool is_serving_static_files;
        union
        {
            const char *dirname;
        } param;
    } opt;
    struct
    {
        void *user_data;
    } conn_defaults;
    void *tcp_server;
} uo_http_server;

uo_http_server *uo_http_server_create(
    const char *port);

void uo_http_server_start(
    uo_http_server *);

bool uo_http_server_set_opt_serve_static_files(
    uo_http_server *,
    const char *dirname);

void uo_http_server_destroy(
    uo_http_server *);

#ifdef __cplusplus
}
#endif

#endif