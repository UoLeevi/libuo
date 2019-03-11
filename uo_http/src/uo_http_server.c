#include "uo_http_server.h"
#include "uo_http_conn.h"
#include "uo_tcp_server.h"
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

static void uo_http_res_404(
    uo_http_res *http_res)
{
    uo_http_res_set_status_line(http_res, UO_HTTP_404, UO_HTTP_1_1);
    uo_http_msg_set_content(http_res, "404 Not Found", "text/plain", UO_STRLEN("404 Not Found"));
}

static void uo_http_server_serve_static_file(
    uo_http_conn *http_conn,
    const char *dirname)
{
    uo_http_req *http_req = http_conn->http_req;
    uo_http_res *http_res = http_conn->http_res;

    uo_buf filename_buf = uo_buf_alloc(0x200);
    char *target = uo_http_req_get_uri(http_req);

    if (strcmp(target, "/") == 0)
        target = "/index.html";

    uo_buf_printf_append(&filename_buf, "%s%s", dirname, target);

    if (uo_http_res_set_content_from_file(http_res, filename_buf))
        uo_http_res_set_status_line(http_res, UO_HTTP_200, UO_HTTP_1_1);
    else
        uo_http_res_404(http_res);

    uo_buf_free(filename_buf);
}

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

    bool should_close = false;

    if (http_conn->http_req->headers)
    {
        char *header_connection = uo_strhashtbl_get(http_conn->http_req->headers, "connection");
        should_close &= header_connection && strcmp(header_connection, "close") == 0;
    }

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
    uo_http_msg *http_res = http_conn->http_res;
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;

    if (!http_res->start_line_len)
    {
        uo_http_server *http_server = http_conn->http_server;

        if (http_server->opt.is_serving_static_files)
            uo_http_server_serve_static_file(http_conn, http_server->opt.param.dirname);
        else
            uo_http_res_404(http_res);
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

static void uo_http_server_process_request_handlers(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_msg *http_res = http_conn->http_res;

    if (!http_res->start_line_len)
    {
        uo_http_req *http_req = http_conn->http_req;
        uo_http_server *http_server = http_conn->http_server;

        uo_strhashtbl *handlers;
        uo_cb *handler;

        switch (uo_http_req_get_method(http_req))
        {
            case UO_HTTP_GET:    handlers = http_server->request_handlers.GET;    break;
            case UO_HTTP_POST:   handlers = http_server->request_handlers.POST;   break;
            case UO_HTTP_PUT:    handlers = http_server->request_handlers.PUT;    break;
            case UO_HTTP_DELETE: handlers = http_server->request_handlers.DELETE; break;
            default: handlers = NULL; // method not implemented
        }

        if (handlers && (handler = uo_strhashtbl_get(handlers, uo_http_req_get_uri(http_req))))
            uo_cb_prepend(cb, handler);
    }

    uo_cb_invoke(cb);
}

static void tcp_server_evt_handler_after_recv(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);
    uo_http_conn *http_conn = uo_tcp_conn_get_user_data(tcp_conn, "http_conn");
    uo_http_msg *http_req = http_conn->http_req;

    if (http_req->start_line_len
        || uo_http_msg_parse_start_line(http_req))
    {
        if (http_req->headers && uo_http_msg_parse_body(http_req))
        {
            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_conn);
            uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);
            uo_cb_prepend(cb, uo_http_server_process_request_handlers);
            uo_cb_prepend(cb, http_conn->evt_handlers->after_recv_msg);
        }
        else if (uo_http_msg_parse_headers(http_req))
        {
            uo_cb_stack_pop(cb);
            uo_cb_stack_push(cb, http_conn);
            uo_cb_prepend(cb, uo_http_conn_pass_cb_to_tcp_server);

            if (uo_http_msg_parse_body(http_req))
            {
                uo_cb_prepend(cb, uo_http_server_process_request_handlers);
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

    http_server->request_handlers.GET    = uo_strhashtbl_create(0);
    http_server->request_handlers.POST   = uo_strhashtbl_create(0);
    http_server->request_handlers.PUT    = uo_strhashtbl_create(0);
    http_server->request_handlers.DELETE = uo_strhashtbl_create(0);

    return http_server;
}

bool uo_http_server_set_opt_serve_static_files(
    uo_http_server *http_server,
    const char *dirname)
{
    struct stat sb;
    if (stat(dirname, &sb) == -1 || !S_ISDIR(sb.st_mode))
        return false;

    http_server->opt.param.dirname = dirname;
    http_server->opt.is_serving_static_files = true;

    return true;
}

bool uo_http_server_add_request_handler(
    uo_http_server *http_server,
    uo_http_method method,
    const char *uri,
    uo_cb *handler)
{
    uo_strhashtbl *handlers;

    switch (method)
    {
        case UO_HTTP_GET:    handlers = http_server->request_handlers.GET;    break;
        case UO_HTTP_POST:   handlers = http_server->request_handlers.POST;   break;
        case UO_HTTP_PUT:    handlers = http_server->request_handlers.PUT;    break;
        case UO_HTTP_DELETE: handlers = http_server->request_handlers.DELETE; break;
        default: return false; // method not implemented
    }

    uo_strhashtbl_set(handlers, uri, handler);

    return true;
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
    if (!http_server->user_data)
        return NULL;

    return uo_strhashtbl_get(http_server->user_data, key);
}

void uo_http_server_set_user_data(
    uo_http_server *http_server,
    const char *key,
    const void *user_data)
{
    if (!http_server->user_data)
        http_server->user_data = uo_strhashtbl_create(0);

    uo_strhashtbl_set(http_server->user_data, key, user_data);
}

void uo_http_server_destroy(
    uo_http_server *http_server)
{
    uo_tcp_server_destroy(http_server->tcp_server);

    uo_strhashtbl_destroy(http_server->request_handlers.GET);
    uo_strhashtbl_destroy(http_server->request_handlers.POST);
    uo_strhashtbl_destroy(http_server->request_handlers.PUT);
    uo_strhashtbl_destroy(http_server->request_handlers.DELETE);

    uo_cb_destroy(http_server->evt_handlers.after_open);
    uo_cb_destroy(http_server->evt_handlers.after_recv_headers);
    uo_cb_destroy(http_server->evt_handlers.after_recv_msg);
    uo_cb_destroy(http_server->evt_handlers.before_send_msg);
    uo_cb_destroy(http_server->evt_handlers.after_send_msg);
    uo_cb_destroy(http_server->evt_handlers.before_close);
    uo_cb_destroy(http_server->evt_handlers.after_close);

    if (http_server->user_data)
        uo_strhashtbl_destroy(http_server->user_data);

    free(http_server);
}
