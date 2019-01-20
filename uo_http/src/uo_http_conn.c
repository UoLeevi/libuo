#include "uo_http_conn.h"
#include "uo_tcp_conn.h"
#include "uo_strhashtbl.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uo_http_conn *uo_http_conn_create(
    void *tcp_conn,
    UO_HTTP_CONN_ROLE role)
{
    uo_http_conn *http_conn = calloc(1, sizeof *http_conn);
    http_conn->tcp_conn = tcp_conn;

    http_conn->buf = uo_buf_alloc(0x200);

    switch (http_conn->role = role)
    {
        case UO_HTTP_CONN_ROLE_CLIENT:
            http_conn->http_request = uo_http_request_create(&http_conn->buf);
            http_conn->http_response = uo_http_response_create(&((uo_tcp_conn *)tcp_conn)->rbuf);
            break;

        case UO_HTTP_CONN_ROLE_SERVER:
            http_conn->http_request = uo_http_request_create(&((uo_tcp_conn *)tcp_conn)->rbuf);
            http_conn->http_response = uo_http_response_create(&http_conn->buf);
            break;
    }

    return http_conn;
}

void uo_http_conn_parse_request(
    uo_http_conn *http_conn)
{
    uo_tcp_conn *tcp_conn = http_conn->tcp_conn;
    uo_http_request *http_request = http_conn->http_request;

    if ((http_request->target || uo_http_request_parse_start_line(http_request)) &&
        (http_request->headers || uo_http_request_parse_headers(http_request)) &&
        (http_request->method == UO_HTTP_1_1_METHOD_GET) ||
        (http_request->content_len || uo_http_request_parse_body(http_request)) ||
        (http_request->method == UO_HTTP_1_1_METHOD_HEAD))
        http_request->state.is_fully_parsed = true;
}

void uo_http_conn_reset_request(
    uo_http_conn *http_conn)
{
    uo_http_request_destroy(http_conn->http_request);

    switch (http_conn->role)
    {
        case UO_HTTP_CONN_ROLE_CLIENT:
            uo_buf_set_ptr_abs(http_conn->buf, 0);
            http_conn->http_request = uo_http_request_create(&http_conn->buf);
            break;

        case UO_HTTP_CONN_ROLE_SERVER:
            http_conn->http_request = uo_http_request_create(&((uo_tcp_conn *)http_conn->tcp_conn)->rbuf);
            break;
    }
}

void uo_http_conn_reset_response(
    uo_http_conn *http_conn)
{
    uo_http_response_destroy(http_conn->http_response);

    switch (http_conn->role)
    {
        case UO_HTTP_CONN_ROLE_CLIENT:
            http_conn->http_response = uo_http_response_create(&((uo_tcp_conn *)http_conn->tcp_conn)->rbuf);
            break;

        case UO_HTTP_CONN_ROLE_SERVER:
            uo_buf_set_ptr_abs(http_conn->buf, 0);
            http_conn->http_response = uo_http_response_create(&http_conn->buf);
            break;
    }
}

void *uo_http_conn_get_user_data(
    uo_http_conn *http_conn)
{
    return http_conn->user_data;
}

void uo_http_conn_set_user_data(
    uo_http_conn *http_conn,
    void *user_data)
{
    http_conn->user_data = user_data;
}

void uo_http_conn_destroy(
    uo_http_conn *http_conn)
{
    uo_buf_free(http_conn->buf);
    uo_http_response_destroy(http_conn->http_response);
    uo_http_request_destroy(http_conn->http_request);
    free(http_conn);
}
