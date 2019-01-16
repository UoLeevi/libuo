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

static void uo_tcp_client_evt_handler_send_hello(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf_printf_append(&tcp_conn->wbuf, "hello");

    uo_cb_invoke(cb);
}

static void uo_tcp_client_evt_handler_check_response(
    uo_cb *cb)
{
    uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    pass &= memcmp(tcp_conn->rbuf, "hello", 5) == 0;
    sem_post(&sem);

    uo_cb_invoke(cb);
}

static void uo_tcp_server_evt_handler_echo(
    uo_cb *cb)
{
    uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
    uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);

    uo_buf_memcpy_append(&tcp_conn->wbuf, tcp_conn->rbuf, uo_buf_get_len_before_ptr(tcp_conn->rbuf));

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
    uo_cb_append(tcp_server->evt_handlers.before_send, &uo_tcp_server_evt_handler_echo);
    uo_tcp_server_start(tcp_server);

    uo_tcp_client *tcp_client = uo_tcp_client_create("localhost", "12345");
    uo_cb_append(tcp_client->evt_handlers.before_send, &uo_tcp_client_evt_handler_send_hello);
    uo_cb_append(tcp_client->evt_handlers.after_recv, &uo_tcp_client_evt_handler_check_response);
    uo_tcp_client_connect(tcp_client);

    sem_wait(&sem);
    sem_destroy(&sem);

    return pass ? 0 : 1;
}

