#include "uo_tcp_conn.h"
#include "uo_tcp_client.h"
#include "uo_tcp_server.h"
#include "uo_io.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#define UO_TCP_BUF_SIZE 0x1000

static void uo_tcp_conn_advance(uo_cb *);

static void uo_tcp_conn_destroy(
    uo_tcp_conn *tcp_conn)
{
    shutdown(tcp_conn->sockfd, SHUT_RDWR);
    close(tcp_conn->sockfd);
    uo_buf_free(tcp_conn->rbuf);
    uo_buf_free(tcp_conn->wbuf);
    free(tcp_conn);
}

static void uo_tcp_conn_next_state(
    uo_tcp_conn *tcp_conn,
    char state)
{
    char current_state = '\0';
    if (!atomic_compare_exchange_strong(&tcp_conn->state, &current_state, state) 
        && current_state == 'i' 
        && !atomic_compare_exchange_strong(&tcp_conn->state, &current_state, state))
    {
        uo_cb *cb = uo_cb_create();
        uo_cb_stack_push(cb, tcp_conn);
        uo_tcp_conn_advance(cb);
    }
}

static void uo_tcp_conn_reset_evt_arg(
    uo_tcp_conn *tcp_conn)
{
    memset(&tcp_conn->evt_arg, 0, sizeof tcp_conn->evt_arg);
}

static void uo_tcp_conn_after_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_destroy(tcp_conn);
    uo_cb_invoke(cb);
}

static void uo_tcp_conn_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    close(tcp_conn->sockfd);

    uo_cb_append(cb, tcp_conn->evt_handlers->after_close);
    uo_cb_append(cb, uo_tcp_conn_after_close);

    uo_cb_invoke(cb);
}

static void uo_tcp_conn_after_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf_set_ptr_abs(tcp_conn->rbuf, 0);
    uo_buf_set_ptr_abs(tcp_conn->wbuf, 0);
    uo_tcp_conn_reset_evt_arg(tcp_conn);

    uo_cb_invoke(cb);
}

static void uo_tcp_conn_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    size_t wlen;
    unsigned char *p = tcp_conn->wbuf;
    size_t len = uo_buf_get_len_before_ptr(p);
    int wfd = tcp_conn->sockfd;

    while (len)
    {
        // TODO: handle errors properly
        if ((wlen = uo_io_write(wfd, p, len)) == 0)
        {
            uo_cb_stack_pop(cb); // remove error code
            uo_tcp_conn_next_close(tcp_conn);
            uo_cb_invoke(cb);
            return;
        }

        len -= wlen;
        p += wlen;
    }

    uo_cb_append(cb, tcp_conn->evt_handlers->after_send);
    uo_cb_append(cb, uo_tcp_conn_after_send);
    uo_cb_append(cb, uo_tcp_conn_advance);

    uo_cb_invoke(cb);
}

static void uo_tcp_conn_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    size_t len = (uintptr_t)uo_cb_stack_pop(cb);

    if (len)
    {
        tcp_conn->evt_arg.after_recv.last_recv_len = len;
        uo_buf_set_ptr_rel(tcp_conn->rbuf, len);

        uo_cb_append(cb, tcp_conn->evt_handlers->after_recv);
    }
    else
    {
        // TODO: handle errors properly
        uo_cb_stack_pop(cb); // remove error code
        uo_tcp_conn_next_close(tcp_conn);
    }

    uo_cb_append(cb, uo_tcp_conn_advance);

    uo_cb_invoke(cb);
}

static void uo_tcp_conn_before_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf rbuf = tcp_conn->rbuf;

    if (!uo_buf_get_len_after_ptr(rbuf))
        rbuf = tcp_conn->rbuf = uo_buf_realloc_2x(rbuf);

    uo_cb_prepend(cb, uo_tcp_conn_recv);

    uo_io_read_async(
        tcp_conn->sockfd,
        uo_buf_get_ptr(rbuf),
        uo_buf_get_len_after_ptr(rbuf),
        cb);
}

static void uo_tcp_conn_advance(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    switch (atomic_exchange(&tcp_conn->state, '\0'))
    {
        case 'r':
            uo_cb_append(cb, tcp_conn->evt_handlers->before_recv);
            uo_cb_append(cb, uo_tcp_conn_before_recv);
            break;

        case 's':
            uo_cb_append(cb, tcp_conn->evt_handlers->before_send);
            uo_cb_append(cb, uo_tcp_conn_send);
            break;

        case 'c':
            uo_cb_append(cb, tcp_conn->evt_handlers->before_close);
            uo_cb_append(cb, uo_tcp_conn_close);
            break;

        case '\0':
        {
            char current_state = '\0';
            if (!atomic_compare_exchange_strong(&tcp_conn->state, &current_state, 'i'))
                uo_cb_append(cb, uo_tcp_conn_advance);
        }
    }

    uo_cb_invoke(cb);
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

void uo_tcp_conn_next_recv(
    uo_tcp_conn *tcp_conn)
{
    uo_tcp_conn_next_state(tcp_conn, 'r');
}

void uo_tcp_conn_next_send(
    uo_tcp_conn *tcp_conn)
{
    uo_tcp_conn_next_state(tcp_conn, 's');
}

void uo_tcp_conn_next_close(
    uo_tcp_conn *tcp_conn)
{
    uo_tcp_conn_next_state(tcp_conn, 'c');
}

void uo_tcp_conn_open(
    int sockfd,
    uo_tcp_conn_evt_handlers *evt_handlers,
    void *user_data)
{
    uo_tcp_conn *tcp_conn = calloc(1, sizeof *tcp_conn);
    tcp_conn->user_data = user_data;
    tcp_conn->rbuf = uo_buf_alloc(UO_TCP_BUF_SIZE);
    tcp_conn->wbuf = uo_buf_alloc(UO_TCP_BUF_SIZE);
    tcp_conn->evt_handlers = evt_handlers;
    tcp_conn->sockfd = sockfd;

    atomic_init(&tcp_conn->state, '\0');

    uo_cb *cb = uo_cb_clone(tcp_conn->evt_handlers->after_open);
    uo_cb_stack_push(cb, tcp_conn);

    uo_cb_append(cb, uo_tcp_conn_advance);
    uo_cb_invoke_async(cb);
}

void uo_tcp_conn_before_send_length_prefixed_msg(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf wbuf = tcp_conn->wbuf;

    uint32_t len = uo_buf_get_len_before_ptr(wbuf);

    if (uo_buf_get_len_after_ptr(wbuf) < sizeof len)
        wbuf = tcp_conn->wbuf = uo_buf_realloc_2x(wbuf);

    uo_buf_set_ptr_rel(wbuf, sizeof len);
    memmove(wbuf + sizeof len, wbuf, len);
    *(uint32_t *)wbuf = htonl(len);

    uo_cb_invoke(cb);
}

void uo_tcp_conn_after_recv_length_prefixed_msg(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf rbuf = tcp_conn->rbuf;

    uint32_t len = uo_buf_get_len_before_ptr(rbuf);

    if (len < sizeof(uint32_t))
        uo_tcp_conn_next_close(tcp_conn);
    else if (ntohl(*(uint32_t *)rbuf) > (len -= sizeof(uint32_t)))
        uo_tcp_conn_next_recv(tcp_conn);
    else
    {
        tcp_conn->evt_arg.after_recv.is_msg_fully_received = true;
        memmove(rbuf, rbuf + sizeof len, len);
        uo_buf_set_ptr_rel(rbuf, -sizeof(uint32_t));
    }

    uo_cb_invoke(cb);
}

void uo_tcp_conn_next_recv_cb_func(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

void uo_tcp_conn_next_send_cb_func(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_send(tcp_conn);

    uo_cb_invoke(cb);
}

void uo_tcp_conn_next_close_cb_func(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_close(tcp_conn);

    uo_cb_invoke(cb);
}
