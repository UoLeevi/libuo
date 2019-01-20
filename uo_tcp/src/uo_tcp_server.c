#include "uo_tcp_server.h"
#include "uo_err.h"
#include "uo_sock.h"

#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern void uo_tcp_conn_open(
    int sockfd,
    uo_tcp_conn_evt_handlers *,
    void *user_data);

static void *uo_tcp_server_accept(
    void *arg)
{
    uo_tcp_server *tcp_server = arg;

    while (!tcp_server->is_closing) 
    {
        int sockfd;
        if ((sockfd = accept(tcp_server->sockfd, NULL, NULL)) == -1)
        {
            uo_err("Error while accepting connection.");
            continue;
        }

        uo_tcp_conn_open(sockfd, &tcp_server->evt_handlers, tcp_server->conn_defaults.user_data);
    }

    return NULL;
}

void uo_tcp_server_start(
    uo_tcp_server *tcp_server)
{
    pthread_create(tcp_server->thrd, NULL, uo_tcp_server_accept, tcp_server);
}

uo_tcp_server *uo_tcp_server_create(
    const char *port)
{
    uo_tcp_server *tcp_server = calloc(1, sizeof *tcp_server);

    tcp_server->sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_server->sockfd != -1)
    {
        int opt_IPV6_V6ONLY = false;
        uo_setsockopt(tcp_server->sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_IPV6_V6ONLY, sizeof opt_IPV6_V6ONLY);

        int opt_TCP_NODELAY = true;
        uo_setsockopt(tcp_server->sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY);
    }
    else
        uo_err_goto(err_free, "Unable to create socket");

    char *endptr;
    uint16_t porth = strtoul(port, &endptr, 10);

    struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(porth),
        .sin6_addr = in6addr_any
    };

    if (bind(tcp_server->sockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
        uo_err_goto(err_close, "Unable to bind to socket!");

    if (listen(tcp_server->sockfd, SOMAXCONN) != -1)
    {
        char addrp[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.sin6_addr, addrp, INET6_ADDRSTRLEN);
        uint16_t portp = ntohs(addr.sin6_port);

        printf("Listening on [%s]:%u.\r\n", addrp, portp);
    }
    else
        uo_err_goto(err_close, "Unable to listen on socket!\r\n");

    tcp_server->evt_handlers.after_open   = uo_cb_create();
    tcp_server->evt_handlers.before_recv  = uo_cb_create();
    tcp_server->evt_handlers.after_recv   = uo_cb_create();
    tcp_server->evt_handlers.before_send  = uo_cb_create();
    tcp_server->evt_handlers.after_send   = uo_cb_create();
    tcp_server->evt_handlers.before_close = uo_cb_create();
    tcp_server->evt_handlers.after_close  = uo_cb_create();

    tcp_server->thrd = malloc(sizeof(pthread_t));

    return tcp_server;

err_close:
    close(tcp_server->sockfd);

err_free:
    free(tcp_server);

    return NULL;
}

void uo_tcp_server_destroy(
    uo_tcp_server *tcp_server)
{
    tcp_server->is_closing = true;

    close(tcp_server->sockfd);
    pthread_join(*(pthread_t *)tcp_server->thrd, NULL);

    uo_cb_destroy(tcp_server->evt_handlers.after_open);
    uo_cb_destroy(tcp_server->evt_handlers.before_recv);
    uo_cb_destroy(tcp_server->evt_handlers.after_recv);
    uo_cb_destroy(tcp_server->evt_handlers.before_send);
    uo_cb_destroy(tcp_server->evt_handlers.after_send);
    uo_cb_destroy(tcp_server->evt_handlers.before_close);
    uo_cb_destroy(tcp_server->evt_handlers.after_close);

    free(tcp_server);
}
