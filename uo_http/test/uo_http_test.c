#include "uo_http.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_macro.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <semaphore.h>

// TODO
// Proper tests should be written

static bool pass;
static sem_t sem;

static void http_client_evt_handler_before_send_request(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

static void http_client_evt_handler_after_recv_response(
    uo_cb *cb)
{
    
    uo_cb_invoke(cb);
}

static void http_client_evt_handler_after_close(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
    sem_post(&sem);
}

static void http_server_evt_handler_after_recv_request(
    uo_cb *cb)
{
    uo_cb_invoke(cb);
}

int main(
    int argc, 
    char const **argv)
{
    pass = true;

    pass &= uo_http_init();

    sem_init(&sem, 0, 0);

    uo_http_server *http_server = uo_http_server_create("12345");
    uo_http_server_set_opt_serve_static_files(http_server, "test_content");
    uo_cb_append(http_server->evt_handlers.after_recv_msg, http_server_evt_handler_after_recv_request);
    uo_http_server_start(http_server);

    uo_http_client *http_client = uo_http_client_create("localhost", "12345");
    uo_cb_append(http_client->evt_handlers.before_send_msg, http_client_evt_handler_before_send_request);
    uo_cb_append(http_client->evt_handlers.after_recv_msg, http_client_evt_handler_after_recv_response);
    uo_cb_append(http_client->evt_handlers.after_close, http_client_evt_handler_after_close);
    uo_http_client_connect(http_client);

    sem_wait(&sem);
    sem_destroy(&sem);

    uo_http_client_destroy(http_client);
    uo_http_server_destroy(http_server);

    return pass ? 0 : 1;
}
