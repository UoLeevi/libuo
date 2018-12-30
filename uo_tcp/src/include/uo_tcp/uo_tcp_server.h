#ifndef UO_TCP_SERVER_H
#define UO_TCP_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"
#include "uo_cb.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*uo_tcp_evt_handler)(uo_tcp_conn *, uo_cb *);

typedef struct uo_tcp_server
{
    struct
    {
        uo_tcp_evt_handler after_accept_handler;
        uo_tcp_evt_handler before_recv_handler;
        uo_tcp_evt_handler after_recv_handler;
        uo_tcp_evt_handler before_send_handler;
        uo_tcp_evt_handler after_send_handler;
        uo_tcp_evt_handler after_close_handler;
    } evt;
    struct
    {
        void *user_data;
    } conn_defaults;
    void *listen_thrd;
    void *server_thrd;
    void *conn_queue;
    int sockfd;
    bool is_closing;
} uo_tcp_server;

uo_tcp_server *uo_tcp_server_create(
    const char *port);

void uo_tcp_server_start(
    uo_tcp_server *);

void uo_tcp_server_destroy(
    uo_tcp_server *);

#ifdef __cplusplus
}
#endif

#endif