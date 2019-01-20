#ifndef UO_HTTP_CONN_H
#define UO_HTTP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_http_request.h"
#include "uo_http_response.h"
#include "uo_buf.h"

#include "stdbool.h"
#include "stddef.h"

typedef enum UO_HTTP_CONN_ROLE
{
    UO_HTTP_CONN_ROLE_CLIENT,
    UO_HTTP_CONN_ROLE_SERVER
} UO_HTTP_CONN_ROLE;

typedef struct uo_http_conn
{
    void *user_data;
    void *tcp_conn;
    uo_http_request *http_request;
    uo_http_response *http_response;
    UO_HTTP_CONN_ROLE role;
    uo_buf buf;
} uo_http_conn;

uo_http_conn *uo_http_conn_create(
    void *tcp_conn,
    UO_HTTP_CONN_ROLE role);

void uo_http_conn_parse_request(
    uo_http_conn *);

void uo_http_conn_reset_request(
    uo_http_conn *);

void uo_http_conn_reset_response(
    uo_http_conn *);

void *uo_http_conn_get_user_data(
    uo_http_conn *);

void uo_http_conn_set_user_data(
    uo_http_conn *,
    void *user_data);

void uo_http_conn_destroy(
    uo_http_conn *);

#ifdef __cplusplus
}
#endif

#endif
