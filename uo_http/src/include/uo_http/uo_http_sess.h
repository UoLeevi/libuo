#ifndef UO_HTTP_SESS_H
#define UO_HTTP_SESS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_http_msg.h"
#include "uo_cb.h"
#include "uo_buf.h"

#include "stdbool.h"
#include "stddef.h"

typedef struct uo_tcp_conn uo_tcp_conn;
typedef struct uo_http_client uo_http_client;
typedef struct uo_http_server uo_http_server;

typedef struct uo_http_sess_evt_handlers
{
    uo_cb *after_open;
    uo_cb *after_recv_headers;
    uo_cb *after_recv_msg;
    uo_cb *before_send_msg;
    uo_cb *after_send_msg;
    uo_cb *before_close;
    uo_cb *after_close;
} uo_http_sess_evt_handlers;

typedef struct uo_http_sess
{
    void *user_data;
    uo_http_sess_evt_handlers *evt_handlers;
    uo_http_msg *http_request;
    uo_http_msg *http_response;
    uo_tcp_conn *tcp_conn;
    union
    {
        uo_http_client *http_client;
        uo_http_server *http_server;
    } owner;
    uo_buf buf;
} uo_http_sess;

#define uo_http_sess_create(owner, tcp_conn) _Generic((owner), \
    uo_http_client *: uo_http_sess_create_for_client, \
    uo_http_server *: uo_http_sess_create_for_server)(owner, tcp_conn)

uo_http_sess *uo_http_sess_create_for_client(
    uo_http_client *http_client,
    uo_tcp_conn *tcp_conn);

uo_http_sess *uo_http_sess_create_for_server(
    uo_http_server *http_server,
    uo_tcp_conn *tcp_conn);

void uo_http_sess_reset(
    uo_http_sess *);

void *uo_http_sess_get_user_data(
    uo_http_sess *);

void uo_http_sess_set_user_data(
    uo_http_sess *,
    void *user_data);

void uo_http_sess_destroy(
    uo_http_sess *);

void uo_http_sess_next_close(
    uo_http_sess *);

#ifdef __cplusplus
}
#endif

#endif
