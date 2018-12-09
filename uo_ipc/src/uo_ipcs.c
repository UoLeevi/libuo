#include "uo_ipcs.h"
#include "uo_io.h"
#include "uo_err.h"
#include "uo_queue.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#define UO_IPCS_SND_TIMEO_SEC 10

#define UO_IPCS_BUF_LEN 0x1000

typedef struct uo_ipcconn
{
    char *buf;
    int sockfd;
} uo_ipcconn;

static void uo_ipcconn_resize_buf(
    uo_ipcconn *ipcconn,
    size_t buf_len)
{
    ipcconn->buf = realloc(ipcconn->buf, buf_len);
}

static uo_ipcconn *uo_ipcconn_create(
    int sockfd)
{
    uo_ipcconn *ipcconn = malloc(sizeof *ipcconn);
    ipcconn->sockfd = sockfd;
    ipcconn->buf = malloc(UO_IPCS_BUF_LEN);
    return ipcconn;
}

static void uo_ipcconn_destroy(
    uo_ipcconn *ipcconn)
{
    close(ipcconn->sockfd);
    free(ipcconn->buf);
    free(ipcconn);
}

static void *uo_ipcs_send(
    uo_ipcmsg ipcmsg,
    uo_cb *ipcs_send_cb)
{
    uo_ipcconn *conn = uo_cb_stack_pop(ipcs_send_cb);

    int32_t wlen;
    int32_t len = sizeof(uint32_t) + uo_ipcmsg_get_payload_len(ipcmsg);
    char *p = ipcmsg;
    int wfd = conn->sockfd;
    
    while (len)
    {
        if ((wlen = uo_io_write(wfd, p, len)) <= 0)
        {
            uo_ipcconn_destroy(conn);
            return NULL;
        }
        
        len -= wlen;
        p += wlen;
    }

    uo_ipcs *ipcs = uo_cb_stack_pop(ipcs_send_cb);
    uo_queue_enqueue(ipcs->conn_queue, conn, true);
}

static void *uo_ipcs_recv_rest(
    void *recv_len,
    uo_cb *ipcs_recv_first_cb)
{
    ssize_t len = (uintptr_t)recv_len;
    uo_ipcconn *conn = uo_cb_stack_pop(ipcs_recv_first_cb);

    if (!len)
    {
        uo_ipcconn_destroy(conn);
        return NULL;
    }

    char *p = uo_cb_stack_pop(ipcs_recv_first_cb);
    uint32_t left_len = (uintptr_t)uo_cb_stack_pop(ipcs_recv_first_cb);
    uo_ipcs *ipcs = uo_cb_stack_pop(ipcs_recv_first_cb);

    left_len -= len;

    if (left_len)
    {
        p += len;
        uo_cb *ipcs_recv_rest_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipcs_recv_rest_cb, uo_ipcs_recv_rest);
        uo_cb_stack_push(ipcs_recv_rest_cb, ipcs);
        uo_cb_stack_push(ipcs_recv_rest_cb, (void *)(uintptr_t)left_len);
        uo_cb_stack_push(ipcs_recv_rest_cb, p);
        uo_cb_stack_push(ipcs_recv_rest_cb, conn);

        uo_io_read_async(conn->sockfd, p, left_len, ipcs_recv_rest_cb);
    }
    else
    {
        uo_cb *ipcs_send_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipcs_send_cb, (void *(*)(void *, uo_cb *))uo_ipcs_send);
        uo_cb_stack_push(ipcs_send_cb, ipcs);
        uo_cb_stack_push(ipcs_send_cb, conn);
        ipcs->handle_msg(conn->buf, ipcs_send_cb);
    }

    return NULL;
}

static void *uo_ipcs_recv_first(
    void *recv_len,
    uo_cb *ipcs_recv_first_cb)
{
    ssize_t len = (uintptr_t)recv_len;
    uo_ipcconn *conn = uo_cb_stack_pop(ipcs_recv_first_cb);

    if (len < sizeof(uint32_t))
    {
        uo_ipcconn_destroy(conn);
        return NULL;
    }

    uint32_t payload_len = uo_ipcmsg_get_payload_len(conn->buf);
    uint32_t total_len = payload_len + sizeof(uint32_t);

    if (total_len + 1 > UO_IPCS_BUF_LEN)
        uo_ipcconn_resize_buf(conn, total_len + 1);

    conn->buf[total_len] = '\0';

    uint32_t left_len = total_len - len;
    uo_ipcs *ipcs = uo_cb_stack_pop(ipcs_recv_first_cb);

    if (left_len)
    {
        char *p = conn->buf + len;
        uo_cb *ipcs_recv_rest_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipcs_recv_rest_cb, uo_ipcs_recv_rest);
        uo_cb_stack_push(ipcs_recv_rest_cb, ipcs);
        uo_cb_stack_push(ipcs_recv_rest_cb, (void *)(uintptr_t)left_len);
        uo_cb_stack_push(ipcs_recv_rest_cb, p);
        uo_cb_stack_push(ipcs_recv_rest_cb, conn);

        uo_io_read_async(conn->sockfd, p, left_len, ipcs_recv_rest_cb);
    }
    else
    {
        uo_cb *ipcs_send_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipcs_send_cb, (void *(*)(void *, uo_cb *))uo_ipcs_send);
        uo_cb_stack_push(ipcs_send_cb, ipcs);
        uo_cb_stack_push(ipcs_send_cb, conn);
        ipcs->handle_msg(conn->buf, ipcs_send_cb);
    }

    return NULL;
}

static void *uo_ipcs_serve(
    void *arg) 
{
    uo_ipcs *ipcs = arg;

    while (!ipcs->is_closing)
    {
        uo_ipcconn *conn = uo_queue_dequeue(ipcs->conn_queue, true);

        uo_cb *ipcs_recv_first_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipcs_recv_first_cb, uo_ipcs_recv_first);
        uo_cb_stack_push(ipcs_recv_first_cb, ipcs);
        uo_cb_stack_push(ipcs_recv_first_cb, conn);

        uo_io_read_async(conn->sockfd, conn->buf, UO_IPCS_BUF_LEN, ipcs_recv_first_cb);
    }
    
    return NULL;
}

static void *uo_ipcs_listen(
    void *arg)
{
    uo_ipcs *ipcs = arg;

    while (!ipcs->is_closing) 
    {
        int sockfd;
        while ((sockfd = accept(ipcs->sockfd, NULL, NULL)) == -1)
            uo_err("Error while accepting connection.");
        
        uo_queue_enqueue(ipcs->conn_queue, uo_ipcconn_create(sockfd), true);
    }

    return NULL;
}

uo_ipcs *uo_ipcs_create(
    char *servname,
    void *(*handle_msg)(uo_ipcmsg, uo_cb *uo_ipcmsg_cb))
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

        struct timeval opt_SO_SNDTIMEO = { .tv_sec = UO_IPCS_SND_TIMEO_SEC };
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
