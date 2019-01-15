#include "uo_tcp_server.h"
#include "uo_io.h"
#include "uo_err.h"
#include "uo_queue.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

static void uo_tcp_server_after_close(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_tcp_conn_destroy(tcp_conn);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_raise_evt_after_close(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.after_close);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_after_close);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_after_send(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf_set_ptr_abs(tcp_conn->rbuf, 0);
    uo_buf_set_ptr_abs(tcp_conn->wbuf, 0);

    uo_queue_enqueue(tcp_server->conn_queue, tcp_conn, true);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_raise_evt_after_send(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.after_send);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_after_send);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_send(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    ssize_t wlen;
    unsigned char *p = tcp_conn->wbuf;
    size_t len = uo_buf_get_len_before_ptr(p);
    int wfd = tcp_conn->sockfd;
    
    // TODO: handle errors properly
    while (len)
    {
        if ((wlen = uo_io_write(wfd, p, len)) <= 0)
        {
            uo_tcp_server_raise_evt_after_close(tcp_server, tcp_conn);
            uo_cb_invoke(cb);
            return;
        }

        len -= wlen;
        p += wlen;
    }

    uo_tcp_server_raise_evt_after_send(tcp_server, tcp_conn);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_raise_evt_before_send(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.before_send);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_send);
    uo_cb_invoke(cb);
}


static void uo_tcp_server_after_recv(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    if (tcp_conn->evt.next_op == UO_TCP_RECV)
    {
        if (!uo_buf_get_len_after_ptr(tcp_conn->rbuf))
            tcp_conn->rbuf = uo_buf_realloc_2x(tcp_conn->rbuf);

        uo_tcp_conn_reset_evt(tcp_conn);
        uo_queue_enqueue(tcp_server->conn_queue, tcp_conn, true);
    }
    else
        uo_tcp_server_raise_evt_before_send(tcp_server, tcp_conn);

    uo_cb_invoke(cb);
}

static void uo_tcp_server_raise_evt_after_recv(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.after_recv);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_after_recv);
    uo_cb_invoke(cb);
}


static void uo_tcp_server_recv(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    size_t len = (uintptr_t)uo_cb_stack_pop(cb);

    // TODO: handle errors properly
    if (!len)
    {
        uo_tcp_server_raise_evt_after_close(tcp_server, tcp_conn);
        uo_cb_invoke(cb);
        return;
    }

    tcp_conn->evt.last_recv_len = len;
    uo_buf_set_ptr_rel(tcp_conn->rbuf, len);

    uo_tcp_server_raise_evt_after_recv(tcp_server, tcp_conn);

    uo_cb_invoke(cb);
}

static void uo_tcp_server_before_recv(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf rbuf = tcp_conn->rbuf;

    uo_io_read_async(
        tcp_conn->sockfd,
        uo_buf_get_ptr(rbuf),
        uo_buf_get_len_after_ptr(rbuf),
        cb);
}

static void uo_tcp_server_raise_evt_before_recv(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.before_recv);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_before_recv);
    uo_cb_append(cb, uo_tcp_server_recv);
    uo_cb_invoke(cb);
}


static void uo_tcp_server_after_accept(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_queue_enqueue(tcp_server->conn_queue, tcp_conn, true);
    uo_cb_invoke(cb);
}

static void uo_tcp_server_raise_evt_after_accept(
    uo_tcp_server *tcp_server,
    uo_tcp_conn *tcp_conn)
{
    uo_cb *cb = uo_cb_clone(tcp_server->evt_handlers.after_accept);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_server_after_accept);
    uo_cb_invoke(cb);
}


static void *uo_tcp_server_serve(
    void *arg) 
{
    uo_tcp_server *tcp_server = arg;

    while (!tcp_server->is_closing)
    {
        uo_tcp_conn *tcp_conn = uo_queue_dequeue(tcp_server->conn_queue, true);

        if (tcp_conn)
            uo_tcp_server_raise_evt_before_recv(tcp_server, tcp_conn);
    }
    
    return NULL;
}

static void *uo_tcp_server_accept(
    void *arg)
{
    uo_tcp_server *tcp_server = arg;

    while (!tcp_server->is_closing) 
    {
        int sockfd;
        if ((sockfd = accept(tcp_server->sockfd, NULL, NULL)) == -1)
        {
            uo_err("Error while accepting connection.");
            continue;
        }

        uo_tcp_conn *tcp_conn = uo_tcp_conn_create(sockfd);
        uo_tcp_conn_set_user_data(tcp_conn, tcp_server->conn_defaults.user_data);

        uo_tcp_server_raise_evt_after_accept(tcp_server, tcp_conn);
    }

    return NULL;
}

void uo_tcp_server_start(
    uo_tcp_server *tcp_server)
{
    pthread_create(tcp_server->server_thrd, NULL, uo_tcp_server_serve, tcp_server);
    pthread_create(tcp_server->listen_thrd, NULL, uo_tcp_server_accept, tcp_server);
}

uo_tcp_server *uo_tcp_server_create(
    const char *port)
{
    uo_tcp_server *tcp_server = calloc(1, sizeof *tcp_server);

    tcp_server->sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_server->sockfd != -1)
    {
        int opt_IPV6_V6ONLY = false;
        uo_setsockopt(tcp_server->sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_IPV6_V6ONLY, sizeof opt_IPV6_V6ONLY);

        int opt_TCP_NODELAY = true;
        uo_setsockopt(tcp_server->sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY);
    }
    else
        uo_err_goto(err_free, "Unable to create socket");

    char *endptr;
    uint16_t porth = strtoul(port, &endptr, 10);

    struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(porth),
        .sin6_addr = in6addr_any
    };

    if (bind(tcp_server->sockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
        uo_err_goto(err_close, "Unable to bind to socket!");

    if (listen(tcp_server->sockfd, SOMAXCONN) != -1)
    {
        char addrp[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.sin6_addr, addrp, INET6_ADDRSTRLEN);
        uint16_t portp = ntohs(addr.sin6_port);

        printf("Listening on [%s]:%u.\r\n", addrp, portp);
    }
    else
        uo_err_goto(err_close, "Unable to listen on socket!\r\n");

    uo_cb *evt_handler_template = uo_cb_create();
    uo_cb_stack_push(evt_handler_template, tcp_server);

    tcp_server->evt_handlers.after_accept = uo_cb_clone(evt_handler_template);
    tcp_server->evt_handlers.before_recv  = uo_cb_clone(evt_handler_template);
    tcp_server->evt_handlers.after_recv   = uo_cb_clone(evt_handler_template);
    tcp_server->evt_handlers.before_send  = uo_cb_clone(evt_handler_template);
    tcp_server->evt_handlers.after_send   = uo_cb_clone(evt_handler_template);
    tcp_server->evt_handlers.after_close  = uo_cb_clone(evt_handler_template);

    uo_cb_destroy(evt_handler_template);

    tcp_server->conn_queue = uo_queue_create(0x100);

    tcp_server->server_thrd = malloc(sizeof(pthread_t));
    tcp_server->listen_thrd = malloc(sizeof(pthread_t));

    return tcp_server;

err_close:
    close(tcp_server->sockfd);

err_free:
    free(tcp_server);

    return NULL;
}

void uo_tcp_server_destroy(
    uo_tcp_server *tcp_server)
{
    tcp_server->is_closing = true;
    close(tcp_server->sockfd);
    pthread_cancel(*(pthread_t *)tcp_server->listen_thrd);
    uo_queue_enqueue(tcp_server->conn_queue, NULL, true);
    pthread_join(*(pthread_t *)tcp_server->server_thrd, NULL);
    uo_queue_destroy(tcp_server->conn_queue);

    uo_cb_destroy(tcp_server->evt_handlers.after_accept);
    uo_cb_destroy(tcp_server->evt_handlers.before_recv);
    uo_cb_destroy(tcp_server->evt_handlers.after_recv);
    uo_cb_destroy(tcp_server->evt_handlers.before_send);
    uo_cb_destroy(tcp_server->evt_handlers.after_send);
    uo_cb_destroy(tcp_server->evt_handlers.after_close);

    free(tcp_server);
}
