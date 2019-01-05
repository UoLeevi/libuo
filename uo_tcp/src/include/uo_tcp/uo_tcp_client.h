#ifndef UO_TCP_CLIENT_H
#define UO_TCP_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"

#include <stddef.h>
#include <stdint.h>

typedef struct uo_tcp_client
{
    struct
    {
        uo_tcp_evt_handler after_connect_handler;
        uo_tcp_evt_handler before_send_handler;
        uo_tcp_evt_handler after_send_handler;
        uo_tcp_evt_handler before_recv_handler;
        uo_tcp_evt_handler after_recv_handler;
        uo_tcp_evt_handler after_close_handler;
    } evt;
    struct
    {
        void *user_data;
    } conn_defaults;
    const char *hostname;
    const char *port;
} uo_tcp_client;

uo_tcp_client *uo_tcp_client_create(
    const char *hostname,
    const char *port);

void uo_tcp_client_connect(
    uo_tcp_client *);

void uo_tcp_client_close(
    uo_tcp_client *);

void uo_tcp_client_destroy(
    uo_tcp_client *);

#ifdef __cplusplus
}
#endif

#endif