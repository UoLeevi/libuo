#include "uo_tcp_client.h"
#include "uo_io.h"
#include "uo_err.h"
#include "uo_queue.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

static void uo_tcp_evt_after_connect(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_evt_before_send(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_evt_after_send(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_evt_before_recv(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_evt_after_recv(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_evt_after_close(uo_tcp_client *, uo_tcp_conn *);

static void uo_tcp_client_after_close(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_conn_destroy(tcp_conn);
}

static void uo_tcp_evt_after_close(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    if (tcp_client->evt.after_close_handler)
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_client);
        uo_cb_stack_push(cb, tcp_conn);
        uo_cb_append(cb, uo_tcp_client_after_close);
        tcp_client->evt.after_close_handler(tcp_conn, cb);
    }
    else
        uo_tcp_conn_destroy(tcp_conn);
}

static void uo_tcp_client_after_send(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);

    uo_buf_set_ptr_abs(tcp_conn->rbuf, 0);
    uo_buf_set_ptr_abs(tcp_conn->wbuf, 0);

    switch (tcp_conn->evt.next_op)
    {
        case UO_TCP_SEND: uo_tcp_evt_before_send(tcp_client, tcp_conn); break;
        case UO_TCP_CLOSE: uo_tcp_evt_after_close(tcp_client, tcp_conn); break;
        default: uo_tcp_evt_before_recv(tcp_client, tcp_conn); break;
    }
}

static void uo_tcp_evt_after_send(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    if (tcp_client->evt.after_send_handler)
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_client);
        uo_cb_stack_push(cb, tcp_conn);
        uo_cb_append(cb, uo_tcp_client_after_send);
        tcp_client->evt.after_send_handler(tcp_conn, cb);
    }
    else
    {
        uo_buf_set_ptr_abs(tcp_conn->rbuf, 0);
        uo_buf_set_ptr_abs(tcp_conn->wbuf, 0);

        uo_tcp_evt_before_recv(tcp_client, tcp_conn);
    }
}

static void uo_tcp_client_send(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);

    ssize_t wlen;
    unsigned char *p = tcp_conn->wbuf;
    size_t len = uo_buf_get_len_before_ptr(p);
    int wfd = tcp_conn->sockfd;
    
    while (len)
    {
        if ((wlen = uo_io_write(wfd, p, len)) <= 0)
        {
            uo_tcp_evt_after_close(tcp_client, tcp_conn);
            return;
        }

        len -= wlen;
        p += wlen;
    }

    uo_tcp_evt_after_send(tcp_client, tcp_conn);
}

static void uo_tcp_evt_before_send(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_create();
    uo_cb_stack_push(cb, tcp_client);
    uo_cb_stack_push(cb, tcp_conn);
    uo_cb_append(cb, uo_tcp_client_send);

    if (tcp_client->evt.before_send_handler)
        tcp_client->evt.before_send_handler(tcp_conn, cb);
    else
        uo_cb_invoke(cb);
}

static void uo_tcp_client_after_recv(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);

    switch (tcp_conn->evt.next_op)
    {
        case UO_TCP_RECV:

            if (!uo_buf_get_len_after_ptr(tcp_conn->rbuf))
                tcp_conn->rbuf = uo_buf_realloc_2x(tcp_conn->rbuf);

            uo_tcp_conn_reset_evt(tcp_conn);
            uo_tcp_evt_before_recv(tcp_client, tcp_conn);
            
            break;

        case UO_TCP_SEND: uo_tcp_evt_before_send(tcp_client, tcp_conn); break;
        default: uo_tcp_evt_after_close(tcp_client, tcp_conn); break;
    }
}

static void uo_tcp_evt_after_recv(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    if (tcp_client->evt.after_recv_handler)
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_client);
        uo_cb_stack_push(cb, tcp_conn);
        uo_cb_append(cb, uo_tcp_client_after_recv);
        tcp_client->evt.after_recv_handler(tcp_conn, cb);
    }
    else
        uo_tcp_evt_after_close(tcp_client, tcp_conn);
}

static void uo_tcp_client_recv(
    uo_cb_stack *stack)
{
    ssize_t len = (uintptr_t)uo_cb_stack_pop(stack);

    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);

    if (len < 1)
    {
        uo_tcp_evt_after_close(tcp_client, tcp_conn);
        return;
    }

    tcp_conn->evt.last_recv_len = len;
    uo_buf_set_ptr_rel(tcp_conn->rbuf, len);

    uo_tcp_evt_after_recv(tcp_client, tcp_conn);
}

static void uo_tcp_client_before_recv(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);
    uo_cb *tcp_recv_cb = uo_cb_stack_pop(stack);

    uo_buf rbuf = tcp_conn->rbuf;

    uo_io_read_async(
        tcp_conn->sockfd, 
        uo_buf_get_ptr(rbuf), 
        uo_buf_get_len_after_ptr(rbuf), 
        tcp_recv_cb);
}

static void uo_tcp_evt_before_recv(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *tcp_recv_cb = uo_cb_create();
    uo_cb_stack_push(&tcp_recv_cb->stack, tcp_client);
    uo_cb_stack_push(&tcp_recv_cb->stack, tcp_conn);
    uo_cb_append(tcp_recv_cb, uo_tcp_client_recv);

    if (tcp_client->evt.before_recv_handler)
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_recv_cb);
        uo_cb_stack_push(cb, tcp_client);
        uo_cb_stack_push(cb, tcp_conn);
        uo_cb_append(cb, uo_tcp_client_before_recv);
        tcp_client->evt.before_recv_handler(tcp_conn, cb);
    }
    else
    {
        uo_buf rbuf = tcp_conn->rbuf;
        uo_io_read_async(
            tcp_conn->sockfd, 
            uo_buf_get_ptr(rbuf), 
            uo_buf_get_len_after_ptr(rbuf), 
            tcp_recv_cb);
    }
}

static void uo_tcp_client_after_connect(
    uo_cb_stack *stack)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(stack);
    uo_tcp_client *tcp_client = uo_cb_stack_pop(stack);

    if (tcp_conn->evt.next_op == UO_TCP_RECV)
        uo_tcp_evt_before_recv(tcp_client, tcp_conn);
    else
        uo_tcp_evt_before_send(tcp_client, tcp_conn);
}

static void uo_tcp_evt_after_connect(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    if (tcp_client->evt.after_connect_handler)
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_client);
        uo_cb_stack_push(cb, tcp_conn);
        uo_cb_append(cb, uo_tcp_client_after_connect);
        tcp_client->evt.after_connect_handler(tcp_conn, cb);
    }
    else
        uo_tcp_evt_before_send(tcp_client, tcp_conn);
}

void uo_tcp_client_connect(
    uo_tcp_client *tcp_client)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    }, *res;

    int s = getaddrinfo(tcp_client->hostname, tcp_client->port, &hints, &res);
    if (s != 0)
        uo_err_goto(err_return, "Unable to get specified service address. getaddrinfo: %s", gai_strerror(s));

    int sockfd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
        uo_err_goto(err_freeaddrinfo, "Unable to create socket.");

    int opt_TCP_NODELAY = true;
    if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
        uo_err("Could not set TCP_NODELAY option.");

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        uo_err_goto(err_close, "Unable to connect socket.");

    freeaddrinfo(res);

    uo_tcp_conn *tcp_conn = uo_tcp_conn_create(sockfd);
    uo_tcp_conn_set_user_data(tcp_conn, tcp_client->conn_defaults.user_data);

    uo_tcp_evt_after_connect(tcp_client, tcp_conn);

err_close:
    close(sockfd);

err_freeaddrinfo:
    freeaddrinfo(res);

err_return:;
}

uo_tcp_client *uo_tcp_client_create(
    const char *hostname,
    const char *port)
{
    uo_tcp_client *tcp_client = calloc(1, sizeof *tcp_client);

    tcp_client->hostname = hostname;
    tcp_client->port = port;

    return tcp_client;
}

void uo_tcp_client_destroy(
    uo_tcp_client *tcp_client)
{
    free(tcp_client);
}
