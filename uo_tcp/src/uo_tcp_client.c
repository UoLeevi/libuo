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

static void uo_tcp_client_raise_evt_after_connect(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_client_raise_evt_before_send(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_client_raise_evt_after_send(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_client_raise_evt_before_recv(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_client_raise_evt_after_recv(uo_tcp_client *, uo_tcp_conn *);
static void uo_tcp_client_raise_evt_after_close(uo_tcp_client *, uo_tcp_conn *);

static void uo_tcp_client_after_close(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_tcp_conn_destroy(tcp_conn);
    uo_cb_invoke(cb);
}

static void uo_tcp_client_raise_evt_after_close(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.after_close);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_after_close);
    uo_cb_invoke(cb);
}

static void uo_tcp_client_after_send(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf_set_ptr_abs(tcp_conn->rbuf, 0);
    uo_buf_set_ptr_abs(tcp_conn->wbuf, 0);

    switch (tcp_conn->evt.next_op)
    {
        case UO_TCP_SEND: uo_tcp_client_raise_evt_before_send(tcp_client, tcp_conn); break;
        case UO_TCP_CLOSE: uo_tcp_client_raise_evt_after_close(tcp_client, tcp_conn); break;
        default: uo_tcp_client_raise_evt_before_recv(tcp_client, tcp_conn); break;
    }

    uo_cb_invoke(cb);
}

static void uo_tcp_client_raise_evt_after_send(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.after_send);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_after_send);
    uo_cb_invoke(cb);
}

static void uo_tcp_client_send(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    size_t wlen;
    unsigned char *p = tcp_conn->wbuf;
    size_t len = uo_buf_get_len_before_ptr(p);
    int wfd = tcp_conn->sockfd;
    
    // TODO: handle errors properly
    while (len)
    {
        if ((wlen = uo_io_write(wfd, p, len)) <= 0)
        {
            uo_tcp_client_raise_evt_after_close(tcp_client, tcp_conn);
            uo_cb_invoke(cb);
            return;
        }

        len -= wlen;
        p += wlen;
    }

    uo_tcp_client_raise_evt_after_send(tcp_client, tcp_conn);
    uo_cb_invoke(cb);
}

static void uo_tcp_client_raise_evt_before_send(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.before_send);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_send);
    uo_cb_invoke(cb);
}


static void uo_tcp_client_after_recv(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    switch (tcp_conn->evt.next_op)
    {
        case UO_TCP_RECV:

            if (!uo_buf_get_len_after_ptr(tcp_conn->rbuf))
                tcp_conn->rbuf = uo_buf_realloc_2x(tcp_conn->rbuf);

            uo_tcp_conn_reset_evt(tcp_conn);
            uo_tcp_client_raise_evt_before_recv(tcp_client, tcp_conn);
            
            break;

        case UO_TCP_SEND: uo_tcp_client_raise_evt_before_send(tcp_client, tcp_conn); break;
        default: uo_tcp_client_raise_evt_after_close(tcp_client, tcp_conn); break;
    }

    uo_cb_invoke(cb);
}

static void uo_tcp_client_raise_evt_after_recv(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.after_recv);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_after_recv);
    uo_cb_invoke(cb);
}


static void uo_tcp_client_recv(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    size_t len = (uintptr_t)uo_cb_stack_pop(cb);

    // TODO: handle errors properly
    if (!len)
    {
        uo_tcp_client_raise_evt_after_close(tcp_client, tcp_conn);
        uo_cb_invoke(cb);
        return;
    }

    tcp_conn->evt.last_recv_len = len;
    uo_buf_set_ptr_rel(tcp_conn->rbuf, len);

    uo_tcp_client_raise_evt_after_recv(tcp_client, tcp_conn);

    uo_cb_invoke(cb);
}

static void uo_tcp_client_before_recv(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf rbuf = tcp_conn->rbuf;

    uo_io_read_async(
        tcp_conn->sockfd,
        uo_buf_get_ptr(rbuf),
        uo_buf_get_len_after_ptr(rbuf),
        cb);
}

static void uo_tcp_client_raise_evt_before_recv(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.before_recv);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_before_recv);
    uo_cb_append(cb, uo_tcp_client_recv);
    uo_cb_invoke(cb);
}


static void uo_tcp_client_after_connect(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    if (tcp_conn->evt.next_op == UO_TCP_RECV)
        uo_tcp_client_raise_evt_before_recv(tcp_client, tcp_conn);
    else
        uo_tcp_client_raise_evt_before_send(tcp_client, tcp_conn);
    
    uo_cb_invoke(cb);
}

static void uo_tcp_client_raise_evt_after_connect(
    uo_tcp_client *tcp_client,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_client->evt_handlers.after_connect);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_client_after_connect);
    uo_cb_invoke(cb);
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

    uo_tcp_client_raise_evt_after_connect(tcp_client, tcp_conn);

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

    uo_cb *evt_handler_template = uo_cb_create();
    uo_cb_stack_push(evt_handler_template, tcp_client);

    tcp_client->evt_handlers.after_connect = uo_cb_clone(evt_handler_template);
    tcp_client->evt_handlers.before_send   = uo_cb_clone(evt_handler_template);
    tcp_client->evt_handlers.after_send    = uo_cb_clone(evt_handler_template);
    tcp_client->evt_handlers.before_recv   = uo_cb_clone(evt_handler_template);
    tcp_client->evt_handlers.after_recv    = uo_cb_clone(evt_handler_template);
    tcp_client->evt_handlers.after_close   = uo_cb_clone(evt_handler_template);

    uo_cb_destroy(evt_handler_template);

    tcp_client->hostname = hostname;
    tcp_client->port = port;

    return tcp_client;
}

void uo_tcp_client_destroy(
    uo_tcp_client *tcp_client)
{
    uo_cb_destroy(tcp_client->evt_handlers.after_connect);
    uo_cb_destroy(tcp_client->evt_handlers.before_send);
    uo_cb_destroy(tcp_client->evt_handlers.after_send);
    uo_cb_destroy(tcp_client->evt_handlers.before_recv);
    uo_cb_destroy(tcp_client->evt_handlers.after_recv);
    uo_cb_destroy(tcp_client->evt_handlers.after_close);

    free(tcp_client);
}
