#include "uo_http_conn.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_tcp_conn.h"
#include "uo_hashtbl.h"

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

    uo_http_msg_create_at(&http_conn->http_req, &http_conn->buf, 
        UO_HTTP_MSG_TYPE_REQUEST, UO_HTTP_MSG_ROLE_SEND);
    uo_http_msg_create_at(&http_conn->http_res, &tcp_conn->rbuf, 
        UO_HTTP_MSG_TYPE_RESPONSE, UO_HTTP_MSG_ROLE_RECV);

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

    uo_http_msg_create_at(&http_conn->http_req, &tcp_conn->rbuf, 
        UO_HTTP_MSG_TYPE_REQUEST, UO_HTTP_MSG_ROLE_RECV);
    uo_http_msg_create_at(&http_conn->http_res, &http_conn->buf, 
        UO_HTTP_MSG_TYPE_RESPONSE, UO_HTTP_MSG_ROLE_SEND);

    return http_conn;
}

void uo_http_conn_reset(
    uo_http_conn *http_conn)
{
    bool is_client = http_conn->http_req.flags.role == UO_HTTP_MSG_ROLE_SEND;

    uo_buf_set_ptr_abs(http_conn->buf, 0);
    uo_http_msg_destroy_at(&http_conn->http_req);
    uo_http_msg_destroy_at(&http_conn->http_res);

    memset(&http_conn->http_req, 0, sizeof http_conn->http_req);
    memset(&http_conn->http_res, 0, sizeof http_conn->http_res);

    if (is_client)
    {
        uo_http_msg_create_at(&http_conn->http_req, &http_conn->buf, 
            UO_HTTP_MSG_TYPE_REQUEST, UO_HTTP_MSG_ROLE_SEND);
        uo_http_msg_create_at(&http_conn->http_res, &http_conn->tcp_conn->rbuf, 
            UO_HTTP_MSG_TYPE_RESPONSE, UO_HTTP_MSG_ROLE_RECV);
    }
    else
    {
        uo_http_msg_create_at(&http_conn->http_req, &http_conn->tcp_conn->rbuf, 
            UO_HTTP_MSG_TYPE_REQUEST, UO_HTTP_MSG_ROLE_RECV);
        uo_http_msg_create_at(&http_conn->http_res, &http_conn->buf, 
            UO_HTTP_MSG_TYPE_RESPONSE, UO_HTTP_MSG_ROLE_SEND);
    }
}

void *uo_http_conn_get_user_data(
    uo_http_conn *http_conn,
    const char *key)
{
    void *user_data = uo_strhashtbl_get(&http_conn->user_data, key);

    return user_data
        ? user_data
        : uo_strhashtbl_get(http_conn->shared_user_data, key);
}

void uo_http_conn_set_user_data(
    uo_http_conn *http_conn,
    const char *key,
    const void *user_data)
{
    uo_strhashtbl_set(&http_conn->user_data, key, user_data);
}

void uo_http_conn_destroy(
    uo_http_conn *http_conn)
{
    uo_buf_free(http_conn->buf);

    uo_http_msg_destroy_at(&http_conn->http_res);
    uo_http_msg_destroy_at(&http_conn->http_req);

    uo_strhashtbl_destroy_at(&http_conn->user_data);

    free(http_conn);
}

void uo_http_conn_next_close(
    uo_http_conn *http_conn)
{
    uo_tcp_conn_next_close(http_conn->tcp_conn);
}
