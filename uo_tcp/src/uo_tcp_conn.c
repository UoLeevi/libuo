#include "uo_tcp_conn.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#define UO_TCP_BUF_SIZE 0x1000

uo_tcp_conn *uo_tcp_conn_create(
    int sockfd,
    void *state)
{
    uo_tcp_conn *tcp_conn = malloc(sizeof *tcp_conn);
    tcp_conn->buf = uo_buf_alloc(UO_TCP_BUF_SIZE);
    tcp_conn->sockfd = sockfd;
    tcp_conn->state = state;
    tcp_conn->evt_data.recv_len = 0;
    return tcp_conn;
}

void uo_tcp_conn_destroy(
    uo_tcp_conn *tcp_conn)
{
    close(tcp_conn->sockfd);
    uo_buf_free(tcp_conn->buf);
    free(tcp_conn);
}
