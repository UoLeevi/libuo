#include "uo_tcp.h"
#include "uo_tcp_client.h"
#include "uo_tcp_server.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <semaphore.h>

static bool pass;
static sem_t sem;

static void tcp_client_evt_handler_send_hello(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf_printf_append(&tcp_conn->wbuf, "hello");

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_check_response(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    if (tcp_conn->evt_arg.after_recv.is_msg_fully_received)
    {
        pass &= memcmp(tcp_conn->rbuf, "hello", 5) == 0;

        uo_tcp_conn_next_close(tcp_conn);
    }

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_close(
    uo_cb *cb)
{
    sem_post(&sem);
    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_open(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_send(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_before_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf_memcpy_append(&tcp_conn->wbuf, tcp_conn->rbuf, uo_buf_get_len_before_ptr(tcp_conn->rbuf));

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    if (tcp_conn->evt_arg.after_recv.is_msg_fully_received)
        uo_tcp_conn_next_send(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_open(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_close(
    uo_cb *cb)
{
    sem_post(&sem);
    uo_cb_invoke(cb);
}

int main(
    int argc, 
    char **argv)
{
    pass = true;

    pass &= uo_tcp_init();

    sem_init(&sem, 0, 0);

    uo_tcp_server *tcp_server = uo_tcp_server_create("12345");
    uo_cb_append(tcp_server->evt_handlers.after_open, tcp_server_evt_handler_after_open);
    uo_cb_append(tcp_server->evt_handlers.after_recv, tcp_server_evt_handler_after_recv);
    uo_cb_append(tcp_server->evt_handlers.before_send, tcp_server_evt_handler_before_send);
    uo_cb_append(tcp_server->evt_handlers.after_send, tcp_server_evt_handler_after_send);
    uo_cb_append(tcp_server->evt_handlers.after_close, tcp_server_evt_handler_after_close);
    uo_tcp_server_set_opt_use_length_prefixed_messages(tcp_server);
    uo_tcp_server_start(tcp_server);

    uo_tcp_client *tcp_client = uo_tcp_client_create("localhost", "12345");
    uo_cb_append(tcp_client->evt_handlers.after_open, tcp_client_evt_handler_after_open);
    uo_cb_append(tcp_client->evt_handlers.before_send, tcp_client_evt_handler_send_hello);
    uo_cb_append(tcp_client->evt_handlers.after_send, tcp_client_evt_handler_after_send);
    uo_cb_append(tcp_client->evt_handlers.after_recv, tcp_client_evt_handler_check_response);
    uo_cb_append(tcp_client->evt_handlers.after_close, tcp_client_evt_handler_after_close);
    uo_tcp_client_set_opt_use_length_prefixed_messages(tcp_client);
    uo_tcp_client_connect(tcp_client);

    sem_wait(&sem);
    sem_wait(&sem);
    sem_destroy(&sem);

    uo_tcp_client_destroy(tcp_client);
    uo_tcp_server_destroy(tcp_server);

    return pass ? 0 : 1;
}

