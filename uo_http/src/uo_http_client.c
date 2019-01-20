#include "uo_http_client.h"
#include "uo_http_conn.h"
#include "uo_http_request.h"
#include "uo_http_response.h"
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

typedef struct uo_http_tcp_conn_user_data
{
    uo_http_conn *http_conn;
    uo_http_client *http_client;
} uo_http_tcp_conn_user_data;

static void uo_http_client_prepare_request_buf(
    uo_http_conn *http_conn)
{
    uo_http_request *http_request = http_conn->http_request;
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;
    uo_buf *buf = &tcp_conn->wbuf;

    if (!http_conn->http_request->method)
        uo_http_request_set_method(http_conn->http_request, UO_HTTP_1_1_METHOD_GET);

    // TODO: handling of the start line
    uo_buf_printf_append(buf, "GET / HTTP/1.1" "\n\r");

    uo_strhashtbl *headers = http_request->headers;
    struct uo_strkvp *header_kvps = headers->items;
    
    for (size_t i = 0; i < headers->capacity; ++i)
        if ((header_kvps + i)->key)
            uo_buf_printf_append(buf, "%s: %s\r\n", (header_kvps + i)->key, (header_kvps + i)->value);

    uo_buf_memcpy_append(buf, "\r\n", UO_STRLEN("\r\n"));

    if (http_request->content_len)
        uo_buf_memcpy_append(buf, *http_request->buf, http_request->content_len);
}

static void uo_http_client_pass_cb_to_tcp_client(
    uo_cb *tcp_cb)
{
    uo_cb_stack_pop(tcp_cb); // remove http_conn
    uo_cb_stack_pop(tcp_cb); // remove http_client

    uo_cb_invoke(tcp_cb);
}

static void uo_http_client_after_close(
    uo_cb *tcp_cb)
{
    uo_http_client *http_client = uo_cb_stack_index(tcp_cb, 1);
    uo_http_conn *http_conn = uo_cb_stack_index(tcp_cb, 2);

    uo_http_conn_destroy(http_conn);

    uo_cb_invoke(tcp_cb);
}

static void tcp_client_evt_handler_after_close(
    uo_cb *tcp_cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(tcp_cb, 0);

    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_client *http_client = tcp_conn_user_data->http_client;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;
    free(tcp_conn_user_data);

    uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
    uo_cb_prepend(tcp_cb, uo_http_client_after_close);
    uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_close);
    uo_cb_stack_push(tcp_cb, http_client);
    uo_cb_stack_push(tcp_cb, http_conn);

    uo_cb_invoke(tcp_cb);
}

static void uo_http_client_after_recv_response(
    uo_cb *tcp_cb)
{
    uo_http_client *http_client = uo_cb_stack_index(tcp_cb, 1);
    uo_http_conn *http_conn = uo_cb_stack_index(tcp_cb, 2);

    uo_http_conn_reset_request(http_conn);
    uo_http_conn_reset_response(http_conn);

    uo_cb_invoke(tcp_cb);
}

static void tcp_client_evt_handler_after_recv(
    uo_cb *tcp_cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(tcp_cb, 0);

    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_client *http_client = tcp_conn_user_data->http_client;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    uo_http_conn_parse_request(http_conn);

    if (!http_conn->http_response->state.is_fully_parsed)
    {
        uo_tcp_conn_next_recv(tcp_conn);

        if (!http_conn->http_response->state.is_recv_headers_evt_raised && http_conn->http_response->headers)
        {
            http_conn->http_response->state.is_recv_headers_evt_raised = true;

            uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
            uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_recv_headers);
            uo_cb_stack_push(tcp_cb, http_client);
            uo_cb_stack_push(tcp_cb, http_conn);
        }
    }
    else
    {
        uo_tcp_conn_next_send(tcp_conn);

        uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
        uo_cb_prepend(tcp_cb, uo_http_client_after_recv_response);
        uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_recv_response);
        uo_cb_stack_push(tcp_cb, http_client);
        uo_cb_stack_push(tcp_cb, http_conn);

        if (!http_conn->http_response->state.is_recv_headers_evt_raised)
        {
            http_conn->http_response->state.is_recv_headers_evt_raised = true;

            uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_recv_headers);
        }
    }

    uo_cb_invoke(tcp_cb);
}

static void tcp_client_evt_handler_after_send(
    uo_cb *tcp_cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(tcp_cb, 0);

    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_client *http_client = tcp_conn_user_data->http_client;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
    uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_send_request);
    uo_cb_stack_push(tcp_cb, http_client);
    uo_cb_stack_push(tcp_cb, http_conn);

    uo_cb_invoke(tcp_cb);
}

static void uo_http_client_before_send_request(
    uo_cb *tcp_cb)
{
    uo_http_client *http_client = uo_cb_stack_index(tcp_cb, 1);
    uo_http_conn *http_conn = uo_cb_stack_index(tcp_cb, 2);

    uo_http_client_prepare_request_buf(http_conn);

    uo_cb_invoke(tcp_cb);
}

static void tcp_client_evt_handler_before_send(
    uo_cb *tcp_cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(tcp_cb, 0);

    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_client *http_client = tcp_conn_user_data->http_client;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
    uo_cb_prepend(tcp_cb, uo_http_client_before_send_request);
    uo_cb_prepend(tcp_cb, http_client->evt_handlers.before_send_request);
    uo_cb_stack_push(tcp_cb, http_client);
    uo_cb_stack_push(tcp_cb, http_conn);

    uo_cb_invoke(tcp_cb);
}

static void tcp_client_evt_handler_after_open(
    uo_cb *tcp_cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(tcp_cb, 0);

    uo_http_client *http_client = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_conn *http_conn = uo_http_conn_create(tcp_conn, UO_HTTP_CONN_ROLE_CLIENT);
    uo_http_tcp_conn_user_data *tcp_conn_user_data = malloc(sizeof *tcp_conn_user_data);
    tcp_conn_user_data->http_conn = http_conn;
    tcp_conn_user_data->http_client = http_client;
    uo_tcp_conn_set_user_data(tcp_conn, tcp_conn_user_data);

    uo_cb_prepend(tcp_cb, uo_http_client_pass_cb_to_tcp_client);
    uo_cb_prepend(tcp_cb, http_client->evt_handlers.after_open);
    uo_cb_stack_push(tcp_cb, http_client);
    uo_cb_stack_push(tcp_cb, http_conn);

    uo_tcp_conn_next_send(tcp_conn);

    uo_cb_invoke(tcp_cb);
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
    uo_cb_append(tcp_client->evt_handlers.after_close, tcp_client_evt_handler_after_close);

    http_client->tcp_client = tcp_client;

    http_client->evt_handlers.after_open          = uo_cb_create();
    http_client->evt_handlers.before_send_request = uo_cb_create();
    http_client->evt_handlers.after_send_request  = uo_cb_create();
    http_client->evt_handlers.after_recv_headers  = uo_cb_create();
    http_client->evt_handlers.after_recv_response = uo_cb_create();
    http_client->evt_handlers.after_close         = uo_cb_create();

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
    uo_cb_destroy(http_client->evt_handlers.before_send_request);
    uo_cb_destroy(http_client->evt_handlers.after_send_request);
    uo_cb_destroy(http_client->evt_handlers.after_recv_headers);
    uo_cb_destroy(http_client->evt_handlers.after_recv_response);
    uo_cb_destroy(http_client->evt_handlers.after_close);

    free(http_client);
}
