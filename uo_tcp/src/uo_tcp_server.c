#include "uo_tcp_server.h"
#include "uo_hashtbl.h"
#include "uo_queue.h"
#include "uo_err.h"
#include "uo_sock.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern uo_tcp_conn *uo_tcp_conn_create_for_server(
    int sockfd,
    uo_tcp_server *);

extern void uo_tcp_conn_open(
    uo_tcp_conn *);

static void uo_tcp_conn_after_close(
    uo_cb *cb)
{
    uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);
    uo_tcp_server *tcp_server = tcp_conn->tcp_server;
    uo_queue_enqueue(tcp_server->closing_conns, (void *)(uintptr_t)tcp_conn->sockfd, true);

    uo_cb_invoke(cb);
}

static void *uo_tcp_server_accept(
    void *arg)
{
    uo_cb_thrd_init();

    uo_tcp_server *tcp_server = arg;

    int sockfd;

    while (true)
    {
        sockfd = accept(tcp_server->sockfd, NULL, NULL);

        if (tcp_server->is_closing)
            break;

        if (sockfd == -1)
        {
            uo_err("Error while accepting connection.");
            continue;
        }

        uo_tcp_conn *tcp_conn = uo_tcp_conn_create_for_server(sockfd, tcp_server);
        uo_tcp_conn_open(tcp_conn);

        uo_inthashtbl_set(&tcp_server->conns, sockfd, tcp_conn);

        while (sockfd = (uintptr_t)uo_queue_dequeue(tcp_server->closing_conns, false))
            if (sockfd != -1)
                uo_inthashtbl_remove(&tcp_server->conns, sockfd);
    }

    while (tcp_server->conns.count)
        if ((sockfd = (uintptr_t)uo_queue_dequeue(tcp_server->closing_conns, true)) != -1)
            uo_inthashtbl_remove(&tcp_server->conns, sockfd);

    uo_cb_thrd_quit();

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

    tcp_server->port = port;

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

    uo_cb_append(tcp_server->evt_handlers.after_close, uo_tcp_conn_after_close);

    tcp_server->closing_conns = uo_queue_create(0x10);

    uo_inthashtbl_create_at(&tcp_server->conns, 0);
    uo_strhashtbl_create_at(&tcp_server->user_data, 0);

    tcp_server->thrd = malloc(sizeof(pthread_t));

    return tcp_server;

err_close:
    close(tcp_server->sockfd);

err_free:
    free(tcp_server);

    return NULL;
}

extern void uo_tcp_conn_before_send_length_prefixed_msg(
    uo_cb *);

extern void uo_tcp_conn_after_recv_length_prefixed_msg(
    uo_cb *);

bool uo_tcp_server_set_opt_use_length_prefixed_messages(
    uo_tcp_server *tcp_server)
{
    uo_cb_append(tcp_server->evt_handlers.before_send, uo_tcp_conn_before_send_length_prefixed_msg);
    uo_cb_prepend(tcp_server->evt_handlers.after_recv, uo_tcp_conn_after_recv_length_prefixed_msg);

    return true;
}

extern void uo_tcp_conn_next_recv_cb_func(
    uo_cb *);

extern void uo_tcp_conn_next_send_cb_func(
    uo_cb *);

extern void uo_tcp_conn_next_close_cb_func(
    uo_cb *);

bool uo_tcp_server_set_opt_use_flow_recv_send_repeat(
    uo_tcp_server *tcp_server)
{
    uo_cb_append(tcp_server->evt_handlers.after_open, uo_tcp_conn_next_recv_cb_func);
    uo_cb_append(tcp_server->evt_handlers.after_recv, uo_tcp_conn_next_send_cb_func);
    uo_cb_append(tcp_server->evt_handlers.after_send, uo_tcp_conn_next_recv_cb_func);

    return true;
}

bool uo_tcp_server_set_opt_use_flow_recv_send_close(
    uo_tcp_server *tcp_server)
{
    uo_cb_append(tcp_server->evt_handlers.after_open, uo_tcp_conn_next_recv_cb_func);
    uo_cb_append(tcp_server->evt_handlers.after_recv, uo_tcp_conn_next_send_cb_func);
    uo_cb_append(tcp_server->evt_handlers.after_send, uo_tcp_conn_next_close_cb_func);

    return true;
}

void *uo_tcp_server_get_user_data(
    uo_tcp_server *tcp_server,
    const char *key)
{
    return uo_strhashtbl_get(&tcp_server->user_data, key);
}

void uo_tcp_server_set_user_data(
    uo_tcp_server *tcp_server,
    const char *key,
    const void *user_data)
{
    uo_strhashtbl_set(&tcp_server->user_data, key, user_data);
}

void uo_tcp_server_destroy(
    uo_tcp_server *tcp_server)
{
    tcp_server->is_closing = true;

    uo_queue_enqueue(tcp_server->closing_conns, (void *)(uintptr_t)-1, true);

    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    }, *res;

    if (getaddrinfo("localhost", tcp_server->port, &hints, &res) == 0)
    {
        int sockfd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd != -1)
        {
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) != -1)
                pthread_join(*(pthread_t *)tcp_server->thrd, NULL);
            else
                pthread_cancel(*(pthread_t *)tcp_server->thrd);

            close(sockfd);
        }

        freeaddrinfo(res);
    }
    else
        pthread_cancel(*(pthread_t *)tcp_server->thrd);

    close(tcp_server->sockfd);

    uo_cb_destroy(tcp_server->evt_handlers.after_open);
    uo_cb_destroy(tcp_server->evt_handlers.before_recv);
    uo_cb_destroy(tcp_server->evt_handlers.after_recv);
    uo_cb_destroy(tcp_server->evt_handlers.before_send);
    uo_cb_destroy(tcp_server->evt_handlers.after_send);
    uo_cb_destroy(tcp_server->evt_handlers.before_close);
    uo_cb_destroy(tcp_server->evt_handlers.after_close);

    uo_inthashtbl_destroy_at(&tcp_server->conns);
    uo_strhashtbl_destroy_at(&tcp_server->user_data);

    uo_queue_destroy(tcp_server->closing_conns);

    free(tcp_server->thrd);
    free(tcp_server);
}
