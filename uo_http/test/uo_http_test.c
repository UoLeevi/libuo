#include "uo_http.h"
#include "uo_http_server.h"
#include "uo_macro.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static void after_close(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_send(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void before_send(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_recv(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void before_recv(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_accept(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= uo_http_init();
    uo_http_server *http_server = uo_http_server_create("80");
    uo_http_server_set_opt_serve_static_files(http_server, "test_content");
    http_server->evt.after_accept_handler = after_accept;
    http_server->evt.before_recv_handler = before_recv;
    http_server->evt.after_recv_handler = after_recv;
    http_server->evt.before_send_handler = before_send;
    http_server->evt.after_send_handler = after_send;
    http_server->evt.after_close_handler = after_close;
    uo_http_server_start(http_server);

    printf("Press 'q' to quit...\n");
    while(getchar() != 'q');

    uo_http_server_destroy(http_server);

    return passed ? 0 : 1;
}
