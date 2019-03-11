#include "uo_tcp_client.h"
#include "uo_strhashtbl.h"
#include "uo_err.h"
#include "uo_sock.h"

#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern void uo_tcp_conn_open(
    int sockfd,
    uo_tcp_conn_evt_handlers *evt_handlers,
    uo_strhashtbl *shared_user_data);

void uo_tcp_client_connect(
    uo_tcp_client *tcp_client)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    }, *res;

    int s = getaddrinfo(tcp_client->hostname, tcp_client->port, &hints, &res);
    if (s != 0)
        uo_err_goto(err_return, "Unable to get specified service address. getaddrinfo: %s", gai_strerror(s));

    int sockfd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
        uo_err_goto(err_freeaddrinfo, "Unable to create socket.");

    int opt_TCP_NODELAY = true;
    if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
        uo_err("Could not set TCP_NODELAY option.");

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        uo_err_goto(err_close, "Unable to connect socket.");

    freeaddrinfo(res);

    uo_tcp_conn_open(sockfd, &tcp_client->evt_handlers, tcp_client->user_data);

    return;

err_close:
    close(sockfd);

err_freeaddrinfo:
    freeaddrinfo(res);

err_return:;
}

uo_tcp_client *uo_tcp_client_create(
    const char *hostname,
    const char *port)
{
    uo_tcp_client *tcp_client = calloc(1, sizeof *tcp_client);

    tcp_client->evt_handlers.after_open   = uo_cb_create();
    tcp_client->evt_handlers.before_send  = uo_cb_create();
    tcp_client->evt_handlers.after_send   = uo_cb_create();
    tcp_client->evt_handlers.before_recv  = uo_cb_create();
    tcp_client->evt_handlers.after_recv   = uo_cb_create();
    tcp_client->evt_handlers.before_close = uo_cb_create();
    tcp_client->evt_handlers.after_close  = uo_cb_create();

    tcp_client->hostname = hostname;
    tcp_client->port = port;

    return tcp_client;
}

extern void uo_tcp_conn_next_recv_cb_func(
    uo_cb *);

extern void uo_tcp_conn_next_send_cb_func(
    uo_cb *);

extern void uo_tcp_conn_next_close_cb_func(
    uo_cb *);

bool uo_tcp_client_set_opt_use_flow_send_recv_repeat(
    uo_tcp_client *tcp_client)
{
    uo_cb_append(tcp_client->evt_handlers.after_open, uo_tcp_conn_next_send_cb_func);
    uo_cb_append(tcp_client->evt_handlers.after_send, uo_tcp_conn_next_recv_cb_func);
    uo_cb_append(tcp_client->evt_handlers.after_recv, uo_tcp_conn_next_send_cb_func);

    return true;
}

bool uo_tcp_client_set_opt_use_flow_send_recv_close(
    uo_tcp_client *tcp_client)
{
    uo_cb_append(tcp_client->evt_handlers.after_open, uo_tcp_conn_next_send_cb_func);
    uo_cb_append(tcp_client->evt_handlers.after_send, uo_tcp_conn_next_recv_cb_func);
    uo_cb_append(tcp_client->evt_handlers.after_recv, uo_tcp_conn_next_close_cb_func);

    return true;
}

extern void uo_tcp_conn_before_send_length_prefixed_msg(
    uo_cb *);

extern void uo_tcp_conn_after_recv_length_prefixed_msg(
    uo_cb *);

bool uo_tcp_client_set_opt_use_length_prefixed_messages(
    uo_tcp_client *tcp_client)
{
    uo_cb_append(tcp_client->evt_handlers.before_send, uo_tcp_conn_before_send_length_prefixed_msg);
    uo_cb_prepend(tcp_client->evt_handlers.after_recv, uo_tcp_conn_after_recv_length_prefixed_msg);

    return true;
}

void *uo_tcp_client_get_user_data(
    uo_tcp_client *tcp_client,
    const char *key)
{
    if (!tcp_client->user_data)
        return NULL;

    return uo_strhashtbl_get(tcp_client->user_data, key);
}

void uo_tcp_client_set_user_data(
    uo_tcp_client *tcp_client,
    const char *key,
    const void *user_data)
{
    if (!tcp_client->user_data)
        tcp_client->user_data = uo_strhashtbl_create(0);

    uo_strhashtbl_set(tcp_client->user_data, key, user_data);
}

void uo_tcp_client_destroy(
    uo_tcp_client *tcp_client)
{
    uo_cb_destroy(tcp_client->evt_handlers.after_open);
    uo_cb_destroy(tcp_client->evt_handlers.before_send);
    uo_cb_destroy(tcp_client->evt_handlers.after_send);
    uo_cb_destroy(tcp_client->evt_handlers.before_recv);
    uo_cb_destroy(tcp_client->evt_handlers.after_recv);
    uo_cb_destroy(tcp_client->evt_handlers.before_close);
    uo_cb_destroy(tcp_client->evt_handlers.after_close);

    if (tcp_client->user_data)
        uo_strhashtbl_destroy(tcp_client->user_data);

    free(tcp_client);
}
