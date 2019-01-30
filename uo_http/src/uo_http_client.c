#include "uo_http_client.h"
#include "uo_http_sess.h"
#include "uo_tcp_client.h"
#include "uo_tcp.h"
#include "uo_strhashtbl.h"
#include "uo_mem.h"
#include "uo_buf.h"
#include "uo_macro.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/stat.h>

static void uo_http_sess_pass_cb_to_tcp_client(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_pop(cb); // remove http_sess
    uo_tcp_conn *tcp_conn = http_sess->tcp_conn;
    uo_cb_stack_push(cb, tcp_conn); // add tcp_conn

    uo_cb_invoke(cb);
}

static void uo_http_client_after_close(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_pop(cb); // remove http_sess
    uo_tcp_conn *tcp_conn = http_sess->tcp_conn;
    uo_cb_stack_push(cb, tcp_conn); // add tcp_conn

    uo_http_sess_destroy(http_sess);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_sess *http_sess = tcp_conn->user_data;

    uo_cb_stack_push(cb, http_sess);

    uo_cb_prepend(cb, uo_http_client_after_close);
    uo_cb_prepend(cb, http_sess->evt_handlers->after_close);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_before_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_sess *http_sess = tcp_conn->user_data;

    uo_cb_stack_push(cb, http_sess);

    uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);
    uo_cb_prepend(cb, http_sess->evt_handlers->before_close);

    uo_cb_invoke(cb);
}

static void uo_http_client_after_recv_response(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);

    uo_http_sess_reset(http_sess);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);
    uo_http_sess *http_sess = tcp_conn->user_data;
    uo_http_msg *http_response = http_sess->http_response;

    if (http_response->start_line_len
        || uo_http_msg_parse_start_line(http_response))
    {
        if (http_response->headers && uo_http_msg_parse_body(http_response))
        {
            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_sess);
            uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);
            uo_cb_prepend(cb, http_sess->evt_handlers->after_recv_msg);
        }
        else if (uo_http_msg_parse_headers(http_response))
        {
            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_sess);
            uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);

            if (uo_http_msg_parse_body(http_response))
                uo_cb_prepend(cb, http_sess->evt_handlers->after_recv_msg);
            else
                uo_tcp_conn_next_recv(tcp_conn);

            uo_cb_prepend(cb, http_sess->evt_handlers->after_recv_headers);
        }
        else
            uo_tcp_conn_next_recv(tcp_conn);
    }
    else
        uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_sess *http_sess = tcp_conn->user_data;

    uo_cb_stack_push(cb, http_sess);

    uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);
    uo_cb_prepend(cb, http_sess->evt_handlers->after_send_msg);

    uo_cb_invoke(cb);
}

static void uo_http_client_before_send_request(
    uo_cb *cb)
{
    uo_http_sess *http_sess = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_request = http_sess->http_request;

    if (!http_request->start_line_len)
        uo_http_msg_set_request_line(http_request, UO_HTTP_GET, "/", UO_HTTP_1_1);

    uo_tcp_conn *tcp_conn = http_sess->tcp_conn;
    uo_http_msg_write_to_buf(http_request, &tcp_conn->wbuf);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_before_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_sess *http_sess = tcp_conn->user_data;

    uo_cb_stack_push(cb, http_sess);

    uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);
    uo_cb_prepend(cb, uo_http_client_before_send_request);
    uo_cb_prepend(cb, http_sess->evt_handlers->before_send_msg);

    uo_cb_invoke(cb);
}

static void tcp_client_evt_handler_after_open(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);

    uo_http_client *http_client = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_sess *http_sess = uo_http_sess_create(http_client, tcp_conn);
    uo_tcp_conn_set_user_data(tcp_conn, http_sess);

    uo_cb_stack_push(cb, http_sess);

    uo_cb_prepend(cb, uo_http_sess_pass_cb_to_tcp_client);
    uo_cb_prepend(cb, http_sess->evt_handlers->after_open);

    uo_cb_invoke(cb);
}

uo_http_client *uo_http_client_create(
    const char *hostname,
    const char *port)
{
    uo_http_client *http_client = calloc(1, sizeof *http_client);

    uo_tcp_client *tcp_client = uo_tcp_client_create(hostname, port);
    tcp_client->conn_defaults.user_data = http_client;
    uo_cb_append(tcp_client->evt_handlers.after_open, tcp_client_evt_handler_after_open);
    uo_cb_append(tcp_client->evt_handlers.before_send, tcp_client_evt_handler_before_send);
    uo_cb_append(tcp_client->evt_handlers.after_send, tcp_client_evt_handler_after_send);
    uo_cb_append(tcp_client->evt_handlers.after_recv, tcp_client_evt_handler_after_recv);
    uo_cb_append(tcp_client->evt_handlers.before_close, tcp_client_evt_handler_before_close);
    uo_cb_append(tcp_client->evt_handlers.after_close, tcp_client_evt_handler_after_close);
    uo_tcp_client_set_opt_use_flow_send_recv_close(tcp_client);
    http_client->tcp_client = tcp_client;

    http_client->evt_handlers.after_open         = uo_cb_create();
    http_client->evt_handlers.before_send_msg    = uo_cb_create();
    http_client->evt_handlers.after_send_msg     = uo_cb_create();
    http_client->evt_handlers.after_recv_headers = uo_cb_create();
    http_client->evt_handlers.after_recv_msg     = uo_cb_create();
    http_client->evt_handlers.before_close       = uo_cb_create();
    http_client->evt_handlers.after_close        = uo_cb_create();

    return http_client;
}

void uo_http_client_connect(
    uo_http_client *http_client)
{
    uo_tcp_client_connect(http_client->tcp_client);
}

void uo_http_client_destroy(
    uo_http_client *http_client)
{
    uo_tcp_client_destroy(http_client->tcp_client);

    uo_cb_destroy(http_client->evt_handlers.after_open);
    uo_cb_destroy(http_client->evt_handlers.after_recv_msg);
    uo_cb_destroy(http_client->evt_handlers.before_send_msg);
    uo_cb_destroy(http_client->evt_handlers.after_send_msg);
    uo_cb_destroy(http_client->evt_handlers.after_recv_headers);
    uo_cb_destroy(http_client->evt_handlers.before_close);
    uo_cb_destroy(http_client->evt_handlers.after_close);

    free(http_client);
}
