#include "uo_http_conn.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_tcp_conn.h"
#include "uo_hashtbl.h"
#include "uo_json.h"
#include "uo_util.h"

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

    uo_strhashtbl_create_at(&http_conn->user_data, 0);
    uo_strhashtbl_create_at(&http_conn->req_data, 0);

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

    uo_strhashtbl_create_at(&http_conn->user_data, 0);
    uo_strhashtbl_create_at(&http_conn->req_data, 0);

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

    uo_strhashtbl_destroy_at(&http_conn->req_data);
    uo_strhashtbl_create_at(&http_conn->req_data, 0);

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

char *uo_http_conn_get_req_data(
    uo_http_conn *http_conn,
    const char *key)
{
    char *data = uo_strhashtbl_get(&http_conn->req_data, key);
    if (data)
        return data;

    uo_http_req *http_req = &http_conn->http_req;
    if (!http_req->body)
        return NULL;

    char *content_type = uo_http_msg_get_header(http_req, "content-type");
    if (!content_type)
        return NULL;

    if (uo_isprefix("application/json", content_type))
    {
        char *json_value = uo_json_find_value(http_req->body, key);
        if (!json_value)
            return NULL;

        char *json_value_end = uo_json_find_end(json_value);
        if (!json_value_end)
            return NULL;

        size_t key_len = strlen(key);
        size_t json_len = json_value_end - json_value;
        char *p = data = malloc(json_len + 1 + key_len + 1);
        uo_refstack_push(&http_req->refstack, p, free);

        memcpy(p, json_value, json_len);

        if (*json_value == '"')
        {
            p = uo_json_decode_utf8(p, json_value, json_len);
            if (!p)
                return NULL;
        }
        else
            p += key_len;

        *p++ = '\0';

        memcpy(p, key, key_len);
        p[key_len] = '\0';

        uo_strhashtbl_set(&http_conn->req_data, p, data);

        return data;
    }

    return NULL;
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
