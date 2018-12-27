#ifndef UO_TCP_CONN_H
#define UO_TCP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_cb.h"
#include "uo_buf.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_tcp_conn
{
    void *state;
    uo_buf buf;
    int sockfd;
    union
    {
        size_t recv_len;
        bool recv_again;
    } evt_data;
} uo_tcp_conn;

typedef void (*uo_tcp_evt_handler)(uo_tcp_conn *, uo_cb *cb);

void uo_tcp_conn_buf_resize(
    uo_tcp_conn *tcp_conn,
    size_t size);

uo_tcp_conn *uo_tcp_conn_create(
    int sockfd,
    void *state);

void uo_tcp_conn_destroy(
    uo_tcp_conn *tcp_conn);

#ifdef __cplusplus
}
#endif

#endif
