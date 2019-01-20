#include "uo_tcp_client.h"
#include "uo_err.h"
#include "uo_sock.h"

#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern void uo_tcp_conn_open(
    int sockfd,
    uo_tcp_conn_evt_handlers *);

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

    uo_tcp_conn_open(sockfd, &tcp_client->evt_handlers);

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

    free(tcp_client);
}
