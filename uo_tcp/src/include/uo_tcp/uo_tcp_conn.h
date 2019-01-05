#ifndef UO_TCP_CONN_H
#define UO_TCP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_cb.h"
#include "uo_buf.h"

#include "stdbool.h"
#include "stddef.h"

typedef enum uo_tcp_op
{
    UO_TCP_DEFAULT,
    UO_TCP_RECV,
    UO_TCP_SEND,
    UO_TCP_CLOSE
} uo_tcp_op;

typedef struct uo_tcp_conn_evt
{
    uo_tcp_op next_op;
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

typedef void (*uo_tcp_evt_handler)(uo_tcp_conn *, uo_cb *);

uo_tcp_conn *uo_tcp_conn_create(
    int sockfd);

void uo_tcp_conn_reset_evt(
    uo_tcp_conn *);

void *uo_tcp_conn_get_user_data(
    uo_tcp_conn *);

void uo_tcp_conn_set_user_data(
    uo_tcp_conn *,
    void *user_data);

void uo_tcp_conn_recv(
    uo_tcp_conn *);

void uo_tcp_conn_send(
    uo_tcp_conn *);

void uo_tcp_conn_close(
    uo_tcp_conn *);

void uo_tcp_conn_destroy(
    uo_tcp_conn *);

#ifdef __cplusplus
}
#endif

#endif
