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

typedef struct uo_tcp_server
{
    void *state;
    void (*connected)(void *state);
    void (*data_received)(void *state, uo_buf, size_t new_data_len, bool *recv_again);
    void (*prepare_response)(void *state, uo_buf, uo_cb *send_response_cb);
    void *listen_thrd;
    void *server_thrd;
    void *conn_queue;
    int sockfd;
    bool is_closing;
} uo_tcp_server;

uo_tcp_server *uo_tcp_server_create(
    const char *port,
    void *state,
    void (*connected)(void *state),
    void (*data_received)(void *state, uo_buf, size_t new_data_len, bool *recv_again),
    void (*prepare_response)(void *state, uo_buf, uo_cb *send_response_cb));

void uo_tcp_server_destroy(
    uo_tcp_server *tcp_server);

#ifdef __cplusplus
}
#endif

#endif