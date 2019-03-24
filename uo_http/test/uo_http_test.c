#include "uo_http.h"
#include "uo_http_client.h"
#include "uo_http_server.h"
#include "uo_macro.h"

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdatomic.h>

#include <semaphore.h>

static void http_client_evt_handler_after_open(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    bool *passed = uo_http_conn_get_user_data(http_conn, "passed");
    atomic_size_t *counter = uo_http_conn_get_user_data(http_conn, "counter");
    size_t number = atomic_fetch_add(counter, 1);
    passed[number] = true;
    uo_http_conn_set_user_data(http_conn, "number", (void *)(uintptr_t)number);
    uo_http_conn_set_user_data(http_conn, "passed", passed + number);

    uo_cb_invoke(cb);
}

static void http_client_evt_handler_before_send_request(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_req *http_req = &http_conn->http_req;

    size_t number = (uintptr_t)uo_http_conn_get_user_data(http_conn, "number");
    switch (number % 3)
    {
        case 0:
            uo_http_req_set_request_line(http_req, UO_HTTP_GET, "/", UO_HTTP_VER_1_1);
            break;

        case 1:
            uo_http_req_set_request_line(http_req, UO_HTTP_PUT, "/qwer/asdf", UO_HTTP_VER_1_1);
            uo_http_req_set_content(http_req, NULL, NULL, 0);
            break;

        case 2:
            uo_http_req_set_request_line(http_req, UO_HTTP_GET, "/asdf/qwer/zxcv", UO_HTTP_VER_1_1);
            break;

        default:
            break;
    }

    uo_cb_invoke(cb);
}

static void http_client_evt_handler_after_recv_response(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_res *http_res = &http_conn->http_res;

    bool *passed = uo_http_conn_get_user_data(http_conn, "passed");

    size_t number = (uintptr_t)uo_http_conn_get_user_data(http_conn, "number");
    switch (number % 3)
    {
        case 0:
            *passed &= http_res->status == UO_HTTP_200;
            *passed &= memcmp("<!DOCTYPE html>", http_res->body, 15) == 0;
            break;

        case 1:
            *passed &= http_res->status == UO_HTTP_204;
            *passed &= http_res->body_len == 0;
            break;

        case 2:
            *passed &= http_res->status == UO_HTTP_400;
            *passed &= memcmp("bad request", http_res->body, http_res->body_len) == 0;
            break;

        default:
            *passed &= false;
            break;
    }

    uo_cb_invoke(cb);
}

static void http_client_evt_handler_after_close(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    sem_t *sem = uo_http_conn_get_user_data(http_conn, "sem");

    uo_cb_invoke(cb);

    sem_post(sem);
}

static void http_server_evt_handler_after_recv_request(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_req *http_req = &http_conn->http_req;

    uo_cb_invoke(cb);
}

static void http_server_req_handler_exact(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_req *http_req = &http_conn->http_req;
    uo_http_res *http_res = &http_conn->http_res;

    bool *passed = uo_http_conn_get_user_data(http_conn, "passed");
    *passed &= strcmp(http_req->method_sp_uri, "PUT /qwer/asdf") == 0;
    uo_http_res_set_status_line(http_res, UO_HTTP_204, http_req->ver); 

    uo_cb_invoke(cb);
}

static void http_server_req_handler_prefix(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    uo_http_req *http_req = &http_conn->http_req;
    uo_http_res *http_res = &http_conn->http_res;

    bool *passed = uo_http_conn_get_user_data(http_conn, "passed");
    *passed &= strcmp(http_req->method_sp_uri, "GET /asdf/qwer/zxcv") == 0;
    *passed &= strcmp(uo_http_conn_get_req_data(http_conn, "var1"), "qwer") == 0;
    uo_http_res_set_status_line(http_res, UO_HTTP_400, http_req->ver);
    uo_http_res_set_content(http_res, "bad request", "text/plain", 11);

    uo_cb_invoke(cb);
}

static void http_server_evt_handler_after_close(
    uo_cb *cb)
{
    uo_http_conn *http_conn = uo_cb_stack_index(cb, 0);
    sem_t *sem = uo_http_conn_get_user_data(http_conn, "sem");

    uo_cb_invoke(cb);

    sem_post(sem);
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;
    bool passed_s = true;
    bool passed_c[4] = { false, false, false, false };

    sem_t sem_c;
    sem_t sem_s;

    atomic_size_t counter;
    atomic_init(&counter, 0);

    sem_init(&sem_c, 0, 0);
    sem_init(&sem_s, 0, 0);

    passed &= uo_http_init();

    uo_http_server *http_server = uo_http_server_create("12345");
    uo_http_server_add_req_handler(http_server, "PUT /qwer/asdf", http_server_req_handler_exact);
    uo_http_server_add_req_handler(http_server, "GET /asdf/{var1}/*", http_server_req_handler_prefix);
    uo_http_server_set_user_data(http_server, "sem", &sem_s);
    uo_http_server_set_user_data(http_server, "passed", &passed_s);
    uo_http_server_set_opt_serve_static_files(http_server, "test_content", 0xA00000);
    uo_cb_append(http_server->evt_handlers.after_recv_msg, http_server_evt_handler_after_recv_request);
    uo_cb_append(http_server->evt_handlers.after_close, http_server_evt_handler_after_close);
    uo_http_server_start(http_server);

    uo_http_client *http_client = uo_http_client_create("localhost", "12345");
    uo_http_client_set_user_data(http_client, "counter", &counter);
    uo_http_client_set_user_data(http_client, "sem", &sem_c);
    uo_http_client_set_user_data(http_client, "passed", passed_c);
    uo_cb_append(http_client->evt_handlers.after_open, http_client_evt_handler_after_open);
    uo_cb_append(http_client->evt_handlers.before_send_msg, http_client_evt_handler_before_send_request);
    uo_cb_append(http_client->evt_handlers.after_recv_msg, http_client_evt_handler_after_recv_response);
    uo_cb_append(http_client->evt_handlers.after_close, http_client_evt_handler_after_close);
    uo_http_client_connect(http_client);
    uo_http_client_connect(http_client);
    uo_http_client_connect(http_client);
    uo_http_client_connect(http_client);

    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 500;

    while (sem_timedwait(&sem_c, &ts) == -1 && errno == EINTR)
        continue;

    while (sem_timedwait(&sem_c, &ts) == -1 && errno == EINTR)
        continue;
    
    while (sem_timedwait(&sem_c, &ts) == -1 && errno == EINTR)
        continue;
    
    while (sem_timedwait(&sem_c, &ts) == -1 && errno == EINTR)
        continue;

    while (sem_timedwait(&sem_s, &ts) == -1 && errno == EINTR)
        continue;

    sem_destroy(&sem_c);
    sem_destroy(&sem_s);

    uo_http_client_destroy(http_client);
    uo_http_server_destroy(http_server);

    passed &= passed_c[0];
    passed &= passed_c[1];
    passed &= passed_c[2];
    passed &= passed_c[3];
    passed &= passed_s;

    return passed ? 0 : 1;
}
