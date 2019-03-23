#include "uo_http_server.h"
#include "uo_http_conn.h"
#include "uo_http_req_handler.h"
#include "uo_tcp_server.h"
#include "uo_tcp.h"
#include "uo_hashtbl.h"
#include "uo_mem.h"
#include "uo_buf.h"
#include "uo_macro.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static void uo_http_conn_pass_cb_to_tcp_server(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_pop(cb); // remove http_conn
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;
    uo_cb_stack_push(cb, tcp_conn); // add tcp_conn

    uo_cb_invoke(cb);
}

static void uo_http_server_after_close(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_pop(cb); // remove http_conn
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;
    uo_cb_stack_push(cb, tcp_conn); // add tcp_conn

    uo_http_conn_destroy(http_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");

    uo_cb_stack_push(cb, http_conn);

    uo_cb_prepend(cb, uo_http_server_after_close);
    uo_cb_prepend(cb, http_conn->evt_handlers->after_close);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_before_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");

    uo_cb_stack_push(cb, http_conn);

    uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
    uo_cb_prepend(cb, http_conn->evt_handlers->before_close);

    uo_cb_invoke(cb);
}

static void uo_http_server_after_send_response(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);

    uo_http_conn_reset(http_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");

    char *header_connection = uo_strhashtbl_get(&http_conn->http_req.headers, "connection");
    bool should_close = header_connection && strcmp(header_connection, "close") == 0;

    if (should_close)
        uo_tcp_conn_next_close(tcp_conn);
    else
        uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_stack_push(cb, http_conn);
    uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
    uo_cb_prepend(cb, uo_http_server_after_send_response);
    uo_cb_prepend(cb, http_conn->evt_handlers->after_send_msg);

    uo_cb_invoke(cb);
}

static void uo_http_server_before_send_response(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = &http_conn->http_res;
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;

    if (!http_res->flags.start_line)
    {
        uo_http_server *http_server = http_conn->http_server;

        if (http_conn->http_req.method == UO_HTTP_GET && http_server->file_server)
            uo_http_file_server_serve(http_server->file_server, http_conn);
        else
        {
            uo_http_res_set_status_line(http_res, UO_HTTP_400, UO_HTTP_VER_1_1);
            uo_http_msg_set_content(http_res, "400 Bad Request", "text/plain", UO_STRLEN("400 Bad Request"));
        }
    }

    uo_http_msg_write_to_buf(http_res, &tcp_conn->wbuf);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_before_send(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");

    uo_cb_stack_push(cb, http_conn);

    uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
    uo_cb_prepend(cb, uo_http_server_before_send_response);
    uo_cb_prepend(cb, http_conn->evt_handlers->before_send_msg);

    uo_cb_invoke(cb);
}

static void uo_http_server_process_req_handlers(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = &http_conn->http_res;

    if (!http_res->flags.start_line)
    {
        uo_http_req *http_req = &http_conn->http_req;
        uo_http_server *http_server = http_conn->http_server;

        const char *method_sp_uri = http_req->method_sp_uri;
        uo_linklist *link = http_req->temp.prev_handler;

        while ((link = link->next) != &http_server->req_handlers)
        {
            uo_http_req_handler *http_req_handler = (uo_http_req_handler *)link;

            if (uo_http_req_handler_try(http_req_handler, method_sp_uri, &http_conn->req_data, http_req->finstack))
            {
                http_req->temp.prev_handler = &http_req_handler->link;
                uo_cb_prepend(cb, uo_http_server_process_req_handlers);
                uo_cb_prepend(cb, http_req_handler->cb);
                break;
            }
        }
    }

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");
    uo_http_msg *http_req = &http_conn->http_req;

    if (http_req->flags.start_line || uo_http_msg_parse_start_line(http_req))
    {
        if (http_req->flags.headers && uo_http_msg_parse_body(http_req))
        {
            // request is fully parsed
            http_req->temp.prev_handler = &http_conn->http_server->req_handlers;

            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_conn);
            uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
            uo_cb_prepend(cb, uo_http_server_process_req_handlers);
            uo_cb_prepend(cb, http_conn->evt_handlers->after_recv_msg);
        }
        else if (uo_http_msg_parse_headers(http_req))
        {
            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_conn);
            uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);

            if (uo_http_msg_parse_body(http_req))
            {
                // request is fully parsed
                http_req->temp.prev_handler = &http_conn->http_server->req_handlers;

                uo_cb_prepend(cb, uo_http_server_process_req_handlers);
                uo_cb_prepend(cb, http_conn->evt_handlers->after_recv_msg);
            }
            else
                uo_tcp_conn_next_recv(tcp_conn);

            uo_cb_prepend(cb, http_conn->evt_handlers->after_recv_headers);
        }
        else
            uo_tcp_conn_next_recv(tcp_conn);
    }
    else
        uo_tcp_conn_next_recv(tcp_conn);

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_open(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_pop(cb);

    uo_http_server *http_server = uo_tcp_conn_get_user_data(tcp_conn, "http_server");
    uo_http_conn *http_conn = uo_http_conn_create(http_server, tcp_conn);

    uo_tcp_conn_set_user_data(tcp_conn, "http_conn", http_conn);

    uo_cb_stack_push(cb, http_conn);

    uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
    uo_cb_prepend(cb, http_conn->evt_handlers->after_open);

    uo_cb_invoke(cb);
}

uo_http_server *uo_http_server_create(
    const char *port)
{
    uo_http_server *http_server = calloc(1, sizeof *http_server);

    uo_tcp_server *tcp_server = uo_tcp_server_create(port);
    uo_tcp_server_set_user_data(tcp_server, "http_server", http_server);
    uo_cb_append(tcp_server->evt_handlers.after_open,   tcp_server_evt_handler_after_open);
    uo_cb_append(tcp_server->evt_handlers.after_recv,   tcp_server_evt_handler_after_recv);
    uo_cb_append(tcp_server->evt_handlers.before_send,  tcp_server_evt_handler_before_send);
    uo_cb_append(tcp_server->evt_handlers.after_send,   tcp_server_evt_handler_after_send);
    uo_cb_append(tcp_server->evt_handlers.before_close, tcp_server_evt_handler_before_close);
    uo_cb_append(tcp_server->evt_handlers.after_close,  tcp_server_evt_handler_after_close);
    uo_tcp_server_set_opt_use_flow_recv_send_repeat(tcp_server);
    http_server->tcp_server = tcp_server;

    http_server->evt_handlers.after_open         = uo_cb_create();
    http_server->evt_handlers.after_recv_headers = uo_cb_create();
    http_server->evt_handlers.after_recv_msg     = uo_cb_create();
    http_server->evt_handlers.before_send_msg    = uo_cb_create();
    http_server->evt_handlers.after_send_msg     = uo_cb_create();
    http_server->evt_handlers.before_close       = uo_cb_create();
    http_server->evt_handlers.after_close        = uo_cb_create();

    uo_strhashtbl_create_at(&http_server->user_data, 0);
    uo_linklist_selflink(&http_server->req_handlers);

    return http_server;
}

bool uo_http_server_set_opt_serve_static_files(
    uo_http_server *http_server,
    const char *dirname,
    size_t cache_size)
{
    return (http_server->file_server = uo_http_file_server_create(dirname, cache_size)) 
        ? true
        : false;
}

void uo__http_server_add_req_cb_handler(
    uo_http_server *http_server,
    const char *req_pattern,
    const uo_cb *handler)
{
    uo_http_req_handler *http_req_handler = uo_http_req_handler_create(req_pattern, handler);
    uo_linklist_link(&http_server->req_handlers, http_req_handler);
}

void uo__http_server_add_req_func_handler(
    uo_http_server *http_server,
    const char *req_pattern,
    uo_cb_func handler)
{
    uo_cb *cb = uo_cb_create();
    uo_cb_append(cb, handler);
    uo__http_server_add_req_cb_handler(http_server, req_pattern, cb);
    uo_cb_destroy(cb);
}

void uo_http_server_start(
    uo_http_server *http_server)
{
    uo_tcp_server_start(http_server->tcp_server);
}

void *uo_http_server_get_user_data(
    uo_http_server *http_server,
    const char *key)
{
    return uo_strhashtbl_get(&http_server->user_data, key);
}

void uo_http_server_set_user_data(
    uo_http_server *http_server,
    const char *key,
    const void *user_data)
{
    uo_strhashtbl_set(&http_server->user_data, key, user_data);
}

void uo_http_server_destroy(
    uo_http_server *http_server)
{
    uo_tcp_server_destroy(http_server->tcp_server);
    uo_strhashtbl_destroy_at(&http_server->user_data);
    uo_cb_destroy(http_server->evt_handlers.after_open);
    uo_cb_destroy(http_server->evt_handlers.after_recv_headers);
    uo_cb_destroy(http_server->evt_handlers.after_recv_msg);
    uo_cb_destroy(http_server->evt_handlers.before_send_msg);
    uo_cb_destroy(http_server->evt_handlers.after_send_msg);
    uo_cb_destroy(http_server->evt_handlers.before_close);
    uo_cb_destroy(http_server->evt_handlers.after_close);

    uo_linklist *head = &http_server->req_handlers;
    uo_linklist *link;

    while (head != (link = head->next))
    {
        uo_linklist_unlink(link);
        uo_http_req_handler_destroy((uo_http_req_handler *)link);
    }

    if (http_server->file_server)
        uo_http_file_server_destroy(http_server->file_server);

    free(http_server);
}
