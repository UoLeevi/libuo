#ifndef UO_HTTP_CONN_H
#define UO_HTTP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_http_msg.h"
#include "uo_cb.h"
#include "uo_buf.h"

#include "stdbool.h"
#include "stddef.h"

typedef struct uo_http_client uo_http_client;
typedef struct uo_http_server uo_http_server;
typedef struct uo_tcp_conn uo_tcp_conn;
typedef struct uo_strhashtbl uo_strhashtbl;

typedef struct uo_http_conn_evt_handlers
{
    uo_cb *after_open;
    uo_cb *after_recv_headers;
    uo_cb *after_recv_msg;
    uo_cb *before_send_msg;
    uo_cb *after_send_msg;
    uo_cb *before_close;
    uo_cb *after_close;
} uo_http_conn_evt_handlers;

typedef struct uo_http_conn
{
    uo_strhashtbl user_data;
    uo_strhashtbl req_data;
    uo_http_req http_req;
    uo_http_res http_res;
    union
    {
        uo_strhashtbl *shared_user_data;
        uo_http_client *http_client;
        uo_http_server *http_server;
    };
    uo_http_conn_evt_handlers *evt_handlers;
    uo_tcp_conn *tcp_conn;
    uo_buf buf;
} uo_http_conn;

#define uo_http_conn_create(owner, tcp_conn) _Generic((owner), \
    uo_http_client *: uo_http_conn_create_for_client, \
    uo_http_server *: uo_http_conn_create_for_server)(owner, tcp_conn)

uo_http_conn *uo_http_conn_create_for_client(
    uo_http_client *http_client,
    uo_tcp_conn *tcp_conn);

uo_http_conn *uo_http_conn_create_for_server(
    uo_http_server *http_server,
    uo_tcp_conn *tcp_conn);

void uo_http_conn_reset(
    uo_http_conn *);

/**
 * @brief get a pointer to the request data that has been scanned from request URI or from request body
 * 
 * @param key       null terminated string key
 * @return char *   a pointer to the request data
 */
char *uo_http_conn_get_req_data(
    uo_http_conn *http_conn,
    const char *key);

/**
 * @brief get a pointer to the user data that has been set using uo_http_conn_set_user_data
 * 
 * If there is no user_data set for the http_conn then user_data would be checked for
 * the uo_tcp_server or uo_tcp_client that opened the connection.
 * 
 * @param key       null terminated string key
 * @return void *   a pointer to the user data
 */
void *uo_http_conn_get_user_data(
    uo_http_conn *,
    const char *key);

/**
 * @brief store an arbitrary pointer that can be later accessed using uo_http_conn_set_user_data
 * 
 * The stored user_data is specific to the connection that is specified as the first argument.
 * 
 * @param key           null terminated string key
 * @param user_data     a pointer to arbitrary user data
 */
void uo_http_conn_set_user_data(
    uo_http_conn *,
    const char *key,
    const void *user_data);

void uo_http_conn_destroy(
    uo_http_conn *);

void uo_http_conn_next_close(
    uo_http_conn *);

#ifdef __cplusplus
}
#endif

#endif
