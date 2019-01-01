#include "uo_http_server.h"
#include "uo_http_conn.h"
#include "uo_http_request.h"
#include "uo_http_response.h"
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

typedef struct uo_http_tcp_conn_user_data
{
    uo_http_conn *http_conn;
    uo_http_server *http_server;
} uo_http_tcp_conn_user_data;

static void uo_http_server_serve_static_file(
    uo_http_conn *http_conn,
    const char *dirname)
{
    uo_http_request *http_request = http_conn->http_request;
    uo_http_response *http_response = http_conn->http_response;

    uo_buf filename_buf = uo_buf_alloc(0x100);
    char *target = uo_http_request_get_target(http_request);

    if (strcmp(target, "/") == 0)
        target = "/index.html";

    uo_buf_printf_append(&filename_buf, "%s/%s", dirname, target);

    if (uo_http_response_set_file(http_response, filename_buf))
        uo_http_response_set_status(http_response, UO_HTTP_1_1_STATUS_200);
    else
    {
        uo_http_response_set_status(http_response, UO_HTTP_1_1_STATUS_404);
        uo_http_response_set_content(http_response, "404 Not Found", "text/plain", UO_STRLEN("404 Not Found"));
    }

    uo_buf_free(filename_buf);
}

static void uo_http_server_prepare_response_buf(
    uo_http_conn *http_conn)
{
    uo_http_request *http_request = http_conn->http_request;
    uo_http_response *http_response = http_conn->http_response;
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;
    uo_buf *buf = &tcp_conn->wbuf;

    if (!http_response->status)
        uo_http_response_set_status(http_response, http_response->content_len
            ? UO_HTTP_1_1_STATUS_500
            : UO_HTTP_1_1_STATUS_204);

    uo_http_status_append_line(*buf, http_response->status);

    uo_strhashtbl *headers = http_response->headers;
    struct uo_strkvp *h = headers->items;
    
    for (size_t i = 0; i < headers->capacity; ++i)
        if (h[i].key)
            uo_buf_printf_append(buf, "%s: %s\r\n", h[i].key, h[i].value);

    uo_buf_memcpy_append(buf, "\r\n", UO_STRLEN("\r\n"));

    if (http_response->content_len)
        uo_buf_memcpy_append(buf, http_response->buf, http_response->content_len);
}


static void *uo_http_server_after_close(
    void *arg,
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_pop(cb);
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);
    uo_http_conn_destroy(http_conn);
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_after_close(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_server *http_server = tcp_conn_user_data->http_server;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;
    free(tcp_conn_user_data);
    
    if (http_server->evt.after_close_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_stack_push(cb, http_conn);
        uo_cb_append(cb, uo_http_server_after_close);
        http_server->evt.after_close_handler(http_conn, cb);
    }
    else
    {
        uo_http_conn_destroy(http_conn);
        uo_cb_invoke(tcp_cb, NULL);
    }
}

static void *uo_http_server_after_send(
    void *arg,
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_pop(cb);
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);

    uo_http_conn_reset_request(http_conn);
    uo_http_conn_reset_response(http_conn);
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_after_send(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_server *http_server = tcp_conn_user_data->http_server;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    if (http_server->evt.after_send_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_stack_push(cb, http_conn);
        uo_cb_append(cb, uo_http_server_after_send);
        http_server->evt.after_send_handler(http_conn, cb);
    }
    else
    {
        uo_http_conn_reset_request(http_conn);
        uo_http_conn_reset_response(http_conn);
        uo_cb_invoke(tcp_cb, NULL);
    }
}

static void *uo_http_server_before_send(
    void *arg,
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_pop(cb);
    uo_http_server *http_server = uo_cb_stack_pop(cb);
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);

    if (!http_conn->http_response->status && http_server->opt.is_serving_static_files)
        uo_http_server_serve_static_file(http_conn, http_server->opt.param.dirname);

    uo_http_server_prepare_response_buf(http_conn);
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_before_send(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_server *http_server = tcp_conn_user_data->http_server;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    if (http_server->evt.before_send_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_stack_push(cb, http_server);
        uo_cb_stack_push(cb, http_conn);
        uo_cb_append(cb, uo_http_server_before_send);
        http_server->evt.before_send_handler(http_conn, cb);
    }
    else
    {
        if (!http_conn->http_response->status && http_server->opt.is_serving_static_files)
            uo_http_server_serve_static_file(http_conn, http_server->opt.param.dirname);

        uo_http_server_prepare_response_buf(http_conn);
        uo_cb_invoke(tcp_cb, NULL);
    }
}

static void *uo_http_server_after_recv(
    void *arg,
    uo_cb *cb)
{
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);
    
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_after_recv(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_server *http_server = tcp_conn_user_data->http_server;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    uo_http_conn_parse_request(http_conn);

    if (!http_conn->http_request->is_fully_parsed)
    {
        uo_tcp_conn_recv_again(tcp_conn);
        uo_cb_invoke(tcp_cb, NULL);
    }
    else if (http_server->evt.after_recv_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_append(cb, uo_http_server_after_recv);
        http_server->evt.after_recv_handler(http_conn, cb);
    }
    else
        uo_cb_invoke(tcp_cb, NULL);
}

static void *uo_http_server_before_recv(
    void *arg,
    uo_cb *cb)
{
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);
    
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_before_recv(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_tcp_conn_user_data *tcp_conn_user_data = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_server *http_server = tcp_conn_user_data->http_server;
    uo_http_conn *http_conn = tcp_conn_user_data->http_conn;

    if (http_server->evt.before_recv_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_append(cb, uo_http_server_before_recv);
        http_server->evt.before_recv_handler(http_conn, cb);
    }
    else
        uo_cb_invoke(tcp_cb, NULL);
}

static void *uo_http_server_after_accept(
    void *arg,
    uo_cb *cb)
{
    uo_cb *tcp_cb = uo_cb_stack_pop(cb);
    
    uo_cb_invoke(tcp_cb, NULL);
    return NULL;
}

static void uo_http_evt_after_accept(
    uo_tcp_conn *tcp_conn,
    uo_cb *tcp_cb)
{
    uo_http_server *http_server = uo_tcp_conn_get_user_data(tcp_conn);
    uo_http_conn *http_conn = uo_http_conn_create(tcp_conn);
    uo_http_tcp_conn_user_data *tcp_conn_user_data = malloc(sizeof *tcp_conn_user_data);
    tcp_conn_user_data->http_conn = http_conn;
    tcp_conn_user_data->http_server = http_server;
    uo_tcp_conn_set_user_data(tcp_conn, tcp_conn_user_data);

    if (http_server->evt.after_accept_handler)
    {
        uo_cb *cb = uo_cb_create(UO_CB_OPT_DESTROY);
        
        uo_cb_stack_push(cb, tcp_cb);
        uo_cb_append(cb, uo_http_server_after_accept);
        http_server->evt.after_accept_handler(http_conn, cb);
    }
    else
        uo_cb_invoke(tcp_cb, NULL);
}

uo_http_server *uo_http_server_create(
    const char *port)
{
    uo_http_server *http_server = calloc(1, sizeof *http_server);
    uo_tcp_server *tcp_server = uo_tcp_server_create(port);
    tcp_server->conn_defaults.user_data = http_server;
    tcp_server->evt.after_accept_handler = uo_http_evt_after_accept;
    tcp_server->evt.before_recv_handler = uo_http_evt_before_recv;
    tcp_server->evt.after_recv_handler = uo_http_evt_after_recv;
    tcp_server->evt.before_send_handler = uo_http_evt_before_send;
    tcp_server->evt.after_send_handler = uo_http_evt_after_send;
    tcp_server->evt.after_close_handler = uo_http_evt_after_close;

    http_server->tcp_server = tcp_server;

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

void uo_http_server_start(
    uo_http_server *http_server)
{
    uo_tcp_server_start(http_server->tcp_server);
}

void uo_http_server_destroy(
    uo_http_server *http_server)
{
    uo_tcp_server_destroy(http_server->tcp_server);

    free(http_server);
}
