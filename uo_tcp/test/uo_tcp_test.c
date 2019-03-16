#include "uo_tcp.h"
#include "uo_tcp_client.h"
#include "uo_tcp_server.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <semaphore.h>

const char *lorem = "Lorem ipsum dolor sit amet, choro clita percipit cum te, nulla liber appellantur ad quo. Ubique bonorum fabulas at ius, ubique nostrum ut sea, ne nec dictas possim inciderint. Duo ad epicuri salutatus prodesset, pro sanctus phaedrum antiopam ne, ea mel paulo homero accusata. Elitr viderer habemus nec no, ut ius ancillae periculis, per te eruditi indoctum petentium. Iriure eleifend liberavisse pro in, dolore integre ex pro. Usu corpora quaestio efficiantur te. Mollis inermis consulatu nec no, his ei amet referrentur, no ius quidam delicata. Omnis aliquam recteque ne nam. Eum te ludus postea patrioque. Aeque sensibus mei eu, ne.";

static bool pass;
static sem_t sem;

static void tcp_client_evt_handler_before_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf_printf_append(&tcp_conn->wbuf, "%s", lorem);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    if (tcp_conn->evt_arg.after_recv.is_msg_fully_received)
    {
        pass &= memcmp(tcp_conn->rbuf, lorem, sizeof lorem) == 0;

        uo_tcp_conn_next_close(tcp_conn);
    }

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_close(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
    sem_post(&sem);
}

static void tcp_server_evt_handler_before_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);

    uo_buf_memcpy_append(&tcp_conn->wbuf, tcp_conn->rbuf, uo_buf_get_len_before_ptr(tcp_conn->rbuf));

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_close(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
    sem_post(&sem);
}

int main(
    int argc, 
    char **argv)
{
    pass = true;

    pass &= uo_tcp_init();

    sem_init(&sem, 0, 0);

    uo_tcp_server *tcp_server = uo_tcp_server_create("12345");
    uo_cb_append(tcp_server->evt_handlers.before_send, tcp_server_evt_handler_before_send);
    uo_cb_append(tcp_server->evt_handlers.after_close, tcp_server_evt_handler_after_close);
    uo_tcp_server_set_opt_use_flow_recv_send_repeat(tcp_server);
    uo_tcp_server_set_opt_use_length_prefixed_messages(tcp_server);
    uo_tcp_server_start(tcp_server);

    uo_tcp_client *tcp_client = uo_tcp_client_create("localhost", "12345");
    uo_cb_append(tcp_client->evt_handlers.before_send, tcp_client_evt_handler_before_send);
    uo_cb_append(tcp_client->evt_handlers.after_recv, tcp_client_evt_handler_after_recv);
    uo_cb_append(tcp_client->evt_handlers.after_close, tcp_client_evt_handler_after_close);
    uo_tcp_client_set_opt_use_flow_send_recv_close(tcp_client);
    uo_tcp_client_set_opt_use_length_prefixed_messages(tcp_client);
    uo_tcp_client_connect(tcp_client);

    sem_wait(&sem);
    sem_wait(&sem);
    sem_destroy(&sem);

    uo_tcp_client_destroy(tcp_client);
    uo_tcp_server_destroy(tcp_server);

    return pass ? 0 : 1;
}

