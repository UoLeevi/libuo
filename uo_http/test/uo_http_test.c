#include "uo_http.h"
#include "uo_http_server.h"
#include "uo_macro.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static void after_close(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_send_response(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void before_send_response(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_recv_request(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_recv_headers(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void after_accept(
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

    uo_cb_append(http_server->evt_handlers.after_accept, after_accept);
    uo_cb_append(http_server->evt_handlers.after_recv_headers, after_recv_headers);
    uo_cb_append(http_server->evt_handlers.after_recv_request, after_recv_request);
    uo_cb_append(http_server->evt_handlers.before_send_response, before_send_response);
    uo_cb_append(http_server->evt_handlers.after_send_response, after_send_response);
    uo_cb_append(http_server->evt_handlers.after_close, after_close);

    uo_http_server_start(http_server);

    printf("Press 'q' to quit...\n");
    while(getchar() != 'q');

    uo_http_server_destroy(http_server);

    return passed ? 0 : 1;
}
