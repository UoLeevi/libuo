#ifndef UO_TCP_CONN_H
#define UO_TCP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_buf.h"

#include <stddef.h>

typedef struct uo_tcp_conn
{
    void *state;
    uo_buf buf;
    int sockfd;
} uo_tcp_conn;

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
