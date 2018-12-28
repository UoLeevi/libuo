#include "uo_http_server.h"
#include "uo_http_header.h"
#include "uo_http_request.h"
#include "uo_tcp_server.h"
#include "uo_tcp.h"
#include "uo_mem.h"
#include "uo_macro.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define UO_HTTP_RESPONSE_HEADER_SERVER_LIBUO            UO_HTTP_HEADER_SERVER "libuo http_server" "\r\n"
#define UO_HTTP_RESPONSE_HEADER_CONNECTION_KEEP_ALIVE   UO_HTTP_HEADER_CONNECTION "Keep-Alive" "\r\n"

#define UO_HTTP_RESPONSE_404 \
    "HTTP/1.1 404 Not Found" "\r\n" \
    UO_HTTP_RESPONSE_HEADER_SERVER_LIBUO \
    UO_HTTP_RESPONSE_HEADER_CONNECTION_KEEP_ALIVE \
    UO_HTTP_HEADER_CONTENT_TYPE_TEXT \
    UO_HTTP_HEADER_CONTENT_LENGTH "27" "\r\n" \
    "\r\n" \
    "404 Error: Page not found" "\r\n"

#define UO_HTTP_RESPONSE_200_PARTIAL \
    "HTTP/1.1 200 OK" "\r\n" \
    UO_HTTP_RESPONSE_HEADER_SERVER_LIBUO \
    UO_HTTP_RESPONSE_HEADER_CONNECTION_KEEP_ALIVE \
    UO_HTTP_HEADER_CONTENT_LENGTH

typedef struct uo_http_sess
{
    uo_http_request *http_request;
    uo_http_server *http_server;
} uo_http_sess;

static uo_http_sess *uo_http_sess_create(
    uo_http_server *http_server)
{
    uo_http_sess *http_sess = malloc(sizeof *http_sess);
    http_sess->http_server = http_server;
    http_sess->http_request = NULL;
    return http_sess;
}

static void uo_http_sess_destroy(
    uo_http_sess *http_sess)
{
    uo_http_request_destroy(http_sess->http_request);
    free(http_sess);
}

static void uo_http_server_after_send(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    uo_buf buf = tcp_conn->buf;
    uo_http_sess *http_sess = tcp_conn->state;

    uo_http_request_destroy(http_sess->http_request);
    http_sess->http_request = NULL;
    
    uo_buf_set_ptr_abs(buf, 0);

    uo_cb_invoke(cb, NULL);
}

static void uo_http_server_before_send(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    uo_buf buf = tcp_conn->buf;
    if (uo_buf_get_size(buf) < 1000)
        buf = tcp_conn->buf = uo_buf_realloc(buf, 1000);

    uo_http_sess *http_sess = tcp_conn->state;

    char *target = http_sess->http_request->start_line.target;
    unsigned char *p = buf;

    if (target && http_sess->http_server->root_dir)
    {
        uo_http_request_parse_path(
            http_sess->http_request,
            http_sess->http_server->root_dir);

        struct stat sb;
        if (stat(buf, &sb) == -1 || !S_ISREG(sb.st_mode))
            goto error_404;

        FILE *fp = fopen(buf, "rb");
        if (!fp)
            goto error_404;

        const char *header_content_type = uo_http_header_content_type_for_path(buf);

        p = uo_mem_append_str_literal(buf, UO_HTTP_RESPONSE_200_PARTIAL);
        p += sprintf(p, "%d" "\r\n", sb.st_size);
        p = uo_mem_append_str(p, header_content_type);
        p = uo_mem_append_str_literal(p, "\r\n");

        size_t total_response_len = sb.st_size + p - buf;

        if (uo_buf_get_size(buf) < total_response_len)
        {
            uo_buf_set_ptr_abs(buf, p - buf);
            buf = tcp_conn->buf = uo_buf_realloc(buf, total_response_len);
            p = uo_buf_get_ptr(buf);
        }

        if (fread(p, sizeof *p, sb.st_size, fp) != sb.st_size || ferror(fp))
            goto error_404;

        fclose(fp);

        p += sb.st_size;
    }
    else
    {
error_404:
        p = uo_mem_append_str_literal(buf, UO_HTTP_RESPONSE_404);
    }
    
    uo_buf_set_ptr_abs(buf, p - buf);

    uo_cb_invoke(cb, NULL);
}

static void uo_http_server_after_recv(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    uo_http_sess *http_sess = tcp_conn->state;

    if (!http_sess->http_request)
    {
        http_sess->http_request = uo_http_request_create();
        uo_http_request_parse_start_line(http_sess->http_request, tcp_conn->buf);
    }

    tcp_conn->evt_data.recv_again = false;
    uo_cb_invoke(cb, NULL);
}

static void uo_http_server_after_accept(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    tcp_conn->state = uo_http_sess_create(tcp_conn->state);
    uo_cb_invoke(cb, NULL);
}

static void uo_http_server_after_close(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    uo_http_sess_destroy(tcp_conn->state);
    uo_cb_invoke(cb, NULL);
}

uo_http_server *uo_http_server_create(
    const char *port)
{
    uo_http_server *http_server = calloc(1, sizeof *http_server);
    uo_tcp_server *tcp_server = uo_tcp_server_create(port, http_server);
    tcp_server->evt.after_recv_handler = uo_http_server_after_recv;
    tcp_server->evt.before_send_handler = uo_http_server_before_send;
    tcp_server->evt.after_send_handler = uo_http_server_after_send;
    tcp_server->evt.after_accept_handler = uo_http_server_after_accept;
    tcp_server->evt.after_close_handler = uo_http_server_after_close;

    http_server->tcp_server = tcp_server;

    return http_server;
}

void uo_http_server_start(
    uo_http_server *http_server)
{
    uo_tcp_server_start(http_server->tcp_server);
}

bool uo_http_server_set_root_dir(
    uo_http_server *http_server,
    const char *root_dir)
{
    struct stat sb;

    http_server->root_dir = strdup(root_dir);

    return stat(http_server->root_dir, &sb) == 0
        && S_ISDIR(sb.st_mode);
}

void uo_http_server_destroy(
    uo_http_server *http_server)
{
    uo_tcp_server_destroy(http_server->tcp_server);

    if (http_server->root_dir)
        free(http_server->root_dir);

    free(http_server);
}
