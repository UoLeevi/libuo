#include "uo_http_sess.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_tcp_conn.h"
#include "uo_strhashtbl.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uo_http_sess *uo_http_sess_create_for_client(
    uo_http_client *http_client,
    uo_tcp_conn *tcp_conn)
{
    uo_http_sess *http_sess = calloc(1, sizeof *http_sess);
    http_sess->owner.http_client = http_client;
    http_sess->user_data = http_client->sess_defaults.user_data;
    http_sess->evt_handlers = &http_client->evt_handlers;
    http_sess->tcp_conn = tcp_conn;

    http_sess->buf = uo_buf_alloc(0x200);

    http_sess->http_request = uo_http_msg_create(&http_sess->buf);
    http_sess->http_response = uo_http_msg_create(&((uo_tcp_conn *)tcp_conn)->rbuf);

    return http_sess;
}

uo_http_sess *uo_http_sess_create_for_server(
    uo_http_server *http_server,
    uo_tcp_conn *tcp_conn)
{
    uo_http_sess *http_sess = calloc(1, sizeof *http_sess);
    http_sess->owner.http_server = http_server;
    http_sess->user_data = http_server->sess_defaults.user_data;
    http_sess->evt_handlers = &http_server->evt_handlers;
    http_sess->tcp_conn = tcp_conn;

    http_sess->buf = uo_buf_alloc(0x200);

    http_sess->http_request = uo_http_msg_create(&((uo_tcp_conn *)tcp_conn)->rbuf);
    http_sess->http_response = uo_http_msg_create(&http_sess->buf);

    return http_sess;
}

void uo_http_sess_reset(
    uo_http_sess *http_sess)
{
    bool is_client = &http_sess->buf == http_sess->http_request->buf;

    uo_buf_set_ptr_abs(http_sess->buf, 0);
    uo_http_msg_destroy(http_sess->http_request);
    uo_http_msg_destroy(http_sess->http_response);

    if (is_client)
    {
        http_sess->http_request = uo_http_msg_create(&http_sess->buf);
        http_sess->http_response = uo_http_msg_create(&((uo_tcp_conn *)http_sess->tcp_conn)->rbuf);
    }
    else
    {
        http_sess->http_request = uo_http_msg_create(&((uo_tcp_conn *)http_sess->tcp_conn)->rbuf);
        http_sess->http_response = uo_http_msg_create(&http_sess->buf);
    }
}

void *uo_http_sess_get_user_data(
    uo_http_sess *http_sess)
{
    return http_sess->user_data;
}

void uo_http_sess_set_user_data(
    uo_http_sess *http_sess,
    void *user_data)
{
    http_sess->user_data = user_data;
}

void uo_http_sess_destroy(
    uo_http_sess *http_sess)
{
    uo_buf_free(http_sess->buf);
    uo_http_msg_destroy(http_sess->http_response);
    uo_http_msg_destroy(http_sess->http_request);
    free(http_sess);
}

void uo_http_sess_next_close(
    uo_http_sess *http_sess)
{
    uo_tcp_conn_next_close(http_sess->tcp_conn);
}
