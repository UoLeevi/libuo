#include "uo_ipcs.h"
#include "uo_err.h"
#include "uo_queue.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#define RECV_TIMEOUT_SEC 10
#define SEND_TIMEOUT_SEC RECV_TIMEOUT_SEC

static void *uo_ipcs_process_msg(
    uo_ipcmsg *msg,
    uo_cb *uo_icpmsg_cb)
{
    uo_ipcconn *conn = uo_cb_stack_pop(uo_icpmsg_cb);
    uo_queue *conn_queue = uo_cb_stack_pop(uo_icpmsg_cb);

    if (msg && msg->data)
        if (send(conn->sockfd, msg->data, msg->data_len, 0) == -1)
            uo_err_goto(err_close, "Error while sending ipc message.");

    if (send(conn->sockfd, (char *)&conn->eom, sizeof conn->eom, 0) == -1)
        uo_err_goto(err_close, "Error while sending ipc message.");

    char c;
    if (recv(conn->sockfd, &c, 1, MSG_PEEK) == 0)
    {
        shutdown(conn->sockfd, SHUT_RDWR);
        close(conn->sockfd);
        free(conn->buf);
        free(conn);
    }
    else
        uo_queue_enqueue(conn_queue, conn, true);

    if (msg && msg->should_free)
        free(msg->data);
    free(msg);

    return NULL;

err_close:
    close(conn->sockfd);
    free(conn->buf);
    free(conn);
    if (msg->should_free)
        free(msg->data);
    free(msg);

    return NULL;
}

static void *uo_ipcs_listen(
    void *arg)
{
    uo_ipcs *ipcs = arg;
    while (!ipcs->is_closing) 
    {
        uo_ipcconn *conn = calloc(1, sizeof *conn);
        conn->buf = malloc(conn->buf_len = 0x100);
        
        while ((conn->sockfd = accept(ipcs->sockfd, NULL, NULL)) == -1)
            uo_err("Error while accepting connection.");
        
        uo_queue_enqueue(ipcs->conn_queue, conn, true);
    }

}

static void *uo_ipcs_serve(
    void *arg) 
{
    uo_ipcs *ipcs = arg;

    while (!ipcs->is_closing)
    {
        uo_ipcconn *conn = uo_queue_dequeue(ipcs->conn_queue, true);

        if (!conn->eom && recv(conn->sockfd, (char *)&conn->eom, sizeof conn->eom, MSG_WAITALL) == -1)
            uo_err_goto(err_close, "Error while receiving ipc message.");
            
        ssize_t len;
        char *p = conn->buf;
        size_t recv_buf_len = conn->buf_len;

        do
        {
            if ((len = recv(conn->sockfd, p, recv_buf_len, 0)) <= 0)
                uo_err_goto(err_close, "Error while receiving ipc message.");

            p += len;
            recv_buf_len -= len;

            if (!recv_buf_len)
            {
                recv_buf_len = conn->buf_len;
                ptrdiff_t pdiff = p - conn->buf;
                conn->buf = realloc(conn->buf, conn->buf_len *= 2);
                p = conn->buf + pdiff;
            }

        }  while (*(uint32_t *)(p - sizeof conn->eom) != conn->eom);

        p -= sizeof conn->eom;
        *p = '\0';

        uo_cb *uo_icpmsg_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_stack_push(uo_icpmsg_cb, ipcs->conn_queue);
        uo_cb_stack_push(uo_icpmsg_cb, conn);
        uo_cb_append(uo_icpmsg_cb, (void *(*)(void *, uo_cb *))uo_ipcs_process_msg);

        uo_ipcmsg *msg = malloc(sizeof *msg);
        msg->data = conn->buf;
        msg->data_len = p - conn->buf;
        msg->should_free = false;
        
        ipcs->handle_msg(msg, uo_icpmsg_cb);

        continue;

err_close:
        close(conn->sockfd);
        free(conn->buf);
        free(conn);
    }
    
    return NULL;
}

uo_ipcs *uo_ipcs_create(
    char *servname,
    void *(*handle_msg)(uo_ipcmsg *, uo_cb *uo_ipcmsg_cb))
{
    uo_ipcs *ipcs = calloc(1, sizeof *ipcs);
    
    ipcs->handle_msg = handle_msg;

	ipcs->sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (ipcs->sockfd != -1)
	{
		int opt_IPV6_V6ONLY = false;
		uo_setsockopt(ipcs->sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_IPV6_V6ONLY, sizeof opt_IPV6_V6ONLY);

		int opt_TCP_NODELAY = true;
		uo_setsockopt(ipcs->sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY);

        struct timeval opt_SO_RCVTIMEO = { .tv_sec = RECV_TIMEOUT_SEC };
		uo_setsockopt(ipcs->sockfd, IPPROTO_IPV6, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO);

        struct timeval opt_SO_SNDTIMEO = { .tv_sec = SEND_TIMEOUT_SEC };
        uo_setsockopt(ipcs->sockfd, IPPROTO_IPV6, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO);
    }
    else
        uo_err_goto(err_free, "Unable to create socket");

    char *endptr;
    uint16_t porth = strtoul(servname, &endptr, 10);

    struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(porth),
        .sin6_addr = in6addr_any
    };

    if (bind(ipcs->sockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
        uo_err_goto(err_close, "Unable to bind to socket!");

    if (listen(ipcs->sockfd, SOMAXCONN) != -1)
    {
        char addrp[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.sin6_addr, addrp, INET6_ADDRSTRLEN);
        uint16_t portp = ntohs(addr.sin6_port);

        printf("Listening on [%s]:%u.\r\n", addrp, portp);
    }
    else
        uo_err_goto(err_close, "Unable to listen on socket!\r\n");

    ipcs->conn_queue = uo_queue_create(0x100);

    ipcs->server_thrd = malloc(sizeof(pthread_t));
    pthread_create(ipcs->server_thrd, NULL, uo_ipcs_serve, ipcs);

    ipcs->listen_thrd = malloc(sizeof(pthread_t));
    pthread_create(ipcs->listen_thrd, NULL, uo_ipcs_listen, ipcs);

    return ipcs;

err_close:
    close(ipcs->sockfd);

err_free:
    free(ipcs);

    return NULL;
}
