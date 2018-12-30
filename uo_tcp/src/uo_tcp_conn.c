#include "uo_tcp_conn.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#define UO_TCP_BUF_SIZE 0x1000


uo_tcp_conn *uo_tcp_conn_create(
    int sockfd)
{
    uo_tcp_conn *tcp_conn = calloc(1, sizeof *tcp_conn);
    tcp_conn->rbuf = uo_buf_alloc(UO_TCP_BUF_SIZE);
    tcp_conn->wbuf = uo_buf_alloc(UO_TCP_BUF_SIZE);
    tcp_conn->sockfd = sockfd;
    return tcp_conn;
}

void uo_tcp_conn_reset_evt(
    uo_tcp_conn *tcp_conn)
{
    memset(&tcp_conn->evt, 0, sizeof tcp_conn->evt);
}

void *uo_tcp_conn_get_user_data(
    uo_tcp_conn *tcp_conn)
{
    return tcp_conn->user_data;
}

void uo_tcp_conn_set_user_data(
    uo_tcp_conn *tcp_conn,
    void *user_data)
{
    tcp_conn->user_data = user_data;
}

void uo_tcp_conn_recv_again(
    uo_tcp_conn *tcp_conn)
{
    tcp_conn->evt.recv_again = true;
}

void uo_tcp_conn_destroy(
    uo_tcp_conn *tcp_conn)
{
    close(tcp_conn->sockfd);
    uo_buf_free(tcp_conn->rbuf);
    uo_buf_free(tcp_conn->wbuf);
    free(tcp_conn);
}
