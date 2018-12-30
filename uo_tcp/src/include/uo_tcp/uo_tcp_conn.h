#ifndef UO_TCP_CONN_H
#define UO_TCP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_buf.h"

#include "stdbool.h"
#include "stddef.h"

typedef struct uo_tcp_conn_evt
{
    bool recv_again;
    size_t last_recv_len;
} uo_tcp_conn_evt;

typedef struct uo_tcp_conn
{
    void *user_data;
    uo_buf rbuf;
    uo_buf wbuf;
    uo_tcp_conn_evt evt;
    int sockfd;
} uo_tcp_conn;

uo_tcp_conn *uo_tcp_conn_create(
    int sockfd);

void uo_tcp_conn_reset_evt(
    uo_tcp_conn *);

void *uo_tcp_conn_get_user_data(
    uo_tcp_conn *);

void uo_tcp_conn_set_user_data(
    uo_tcp_conn *,
    void *user_data);

void uo_tcp_conn_recv_again(
    uo_tcp_conn *);

void uo_tcp_conn_destroy(
    uo_tcp_conn *);

#ifdef __cplusplus
}
#endif

#endif
