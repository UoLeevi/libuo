#include "uo_http_conn.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_tcp_conn.h"
#include "uo_strhashtbl.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uo_http_conn *uo_http_conn_create_for_client(
    uo_http_client *http_client,
    uo_tcp_conn *tcp_conn)
{
    uo_http_conn *http_conn = calloc(1, sizeof *http_conn);
    http_conn->http_client = http_client;
    http_conn->evt_handlers = &http_client->evt_handlers;
    http_conn->tcp_conn = tcp_conn;

    http_conn->buf = uo_buf_alloc(0x200);

    http_conn->http_req = uo_http_msg_create(&http_conn->buf);
    http_conn->http_res = uo_http_msg_create(&((uo_tcp_conn *)tcp_conn)->rbuf);

    return http_conn;
}

uo_http_conn *uo_http_conn_create_for_server(
    uo_http_server *http_server,
    uo_tcp_conn *tcp_conn)
{
    uo_http_conn *http_conn = calloc(1, sizeof *http_conn);
    http_conn->http_server = http_server;
    http_conn->evt_handlers = &http_server->evt_handlers;
    http_conn->tcp_conn = tcp_conn;

    http_conn->buf = uo_buf_alloc(0x200);

    http_conn->http_req = uo_http_msg_create(&((uo_tcp_conn *)tcp_conn)->rbuf);
    http_conn->http_res = uo_http_msg_create(&http_conn->buf);

    return http_conn;
}

void uo_http_conn_reset(
    uo_http_conn *http_conn)
{
    bool is_client = &http_conn->buf == http_conn->http_req->buf;

    uo_buf_set_ptr_abs(http_conn->buf, 0);
    uo_http_msg_destroy(http_conn->http_req);
    uo_http_msg_destroy(http_conn->http_res);

    if (is_client)
    {
        http_conn->http_req = uo_http_msg_create(&http_conn->buf);
        http_conn->http_res = uo_http_msg_create(&((uo_tcp_conn *)http_conn->tcp_conn)->rbuf);
    }
    else
    {
        http_conn->http_req = uo_http_msg_create(&((uo_tcp_conn *)http_conn->tcp_conn)->rbuf);
        http_conn->http_res = uo_http_msg_create(&http_conn->buf);
    }
}

void *uo_http_conn_get_user_data(
    uo_http_conn *http_conn,
    const char *key)
{
    void *user_data = NULL;

    if (http_conn->user_data)
        user_data = uo_strhashtbl_get(http_conn->user_data, key);

    if (!user_data && *http_conn->shared_user_data)
        user_data = uo_strhashtbl_get(*http_conn->shared_user_data, key);

    return user_data;
}

void uo_http_conn_set_user_data(
    uo_http_conn *http_conn,
    const char *key,
    const void *user_data)
{
    if (!http_conn->user_data)
        http_conn->user_data = uo_strhashtbl_create(0);

    uo_strhashtbl_set(http_conn->user_data, key, user_data);
}

void uo_http_conn_destroy(
    uo_http_conn *http_conn)
{
    uo_buf_free(http_conn->buf);

    uo_http_msg_destroy(http_conn->http_res);
    uo_http_msg_destroy(http_conn->http_req);

    if (http_conn->user_data)
        uo_strhashtbl_destroy(http_conn->user_data);

    free(http_conn);
}

void uo_http_conn_next_close(
    uo_http_conn *http_conn)
{
    uo_tcp_conn_next_close(http_conn->tcp_conn);
}
