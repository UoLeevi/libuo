#include "uo_ipc.h"
#include "uo_tcp.h"
#include "uo_cb.h"
#include "uo_io.h"
#include "uo_sock.h"
#include "uo_err.h"

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#define UO_IPC_RCV_TIMEO_SEC 10
#define UO_IPC_SND_TIMEO_SEC UO_IPC_RCV_TIMEO_SEC

#define UO_IPC_BUF_LEN 0x1000

static bool is_init;

bool uo_ipc_init(void)
{
    if (is_init)
        return is_init;

    is_init = true;

    is_init &= uo_tcp_init();
    is_init &= uo_cb_init();
    is_init &= uo_sock_init();
    is_init &= uo_io_init();

    return is_init;
}

int uo_ipc_connect(
    char *nodename,
    char *servname)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    }, *res;

    int s = getaddrinfo(nodename, servname, &hints, &res);
    if (s != 0)
        uo_err_return(-1, "Unable to get specified service address. getaddrinfo: %s", gai_strerror(s));

    int sockfd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
        uo_err_goto(err_freeaddrinfo, "Unable to create socket.");

    int opt_TCP_NODELAY = true;
    if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
        uo_err("Could not set TCP_NODELAY option.");

    struct timeval opt_SO_RCVTIMEO = {.tv_sec = UO_IPC_RCV_TIMEO_SEC};
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO) == -1)
        uo_err("Could not set SO_RCVTIMEO option.");

    struct timeval opt_SO_SNDTIMEO = {.tv_sec = UO_IPC_SND_TIMEO_SEC};
    if (uo_setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO) == -1)
        uo_err("Could not set SO_SNDTIMEO option.");

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        uo_err_goto(err_close, "Unable to connect socket.");

    freeaddrinfo(res);

    return sockfd;

err_close:
    close(sockfd);

err_freeaddrinfo:
    freeaddrinfo(res);

    return -1;
}

void uo_ipc_disconnect(
    int fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

static void *uo_ipc_recv_rest(
    void *recv_len,
    uo_cb *ipc_recv_cb)
{
    ssize_t len = (uintptr_t)recv_len;

    int wfd = (uintptr_t)uo_cb_stack_pop(ipc_recv_cb);
    char *p = uo_cb_stack_pop(ipc_recv_cb);
    uint32_t left_len = (uintptr_t)uo_cb_stack_pop(ipc_recv_cb);
    uo_ipcmsg ipcmsg = uo_cb_stack_pop(ipc_recv_cb);
    uo_cb *ipcmsg_cb = uo_cb_stack_pop(ipc_recv_cb);

    if (len <= 0)
    {
        free(ipcmsg);
        uo_cb_invoke_async(ipcmsg_cb, NULL, NULL);
        return NULL;
    }

    left_len -= len;

    if (left_len)
    {
        p += len;
        uo_cb *ipc_recvrest_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipc_recvrest_cb, uo_ipc_recv_rest);
        uo_cb_stack_push(ipc_recvrest_cb, ipcmsg_cb);
        uo_cb_stack_push(ipc_recvrest_cb, ipcmsg);
        uo_cb_stack_push(ipc_recvrest_cb, (void *)(uintptr_t)left_len);
        uo_cb_stack_push(ipc_recvrest_cb, p);
        uo_cb_stack_push(ipc_recvrest_cb, (void *)(uintptr_t)wfd);

        uo_io_read_async(wfd, p, left_len, ipc_recvrest_cb);
    }
    else
        uo_cb_invoke(ipcmsg_cb, ipcmsg);

    return NULL;
}

static void *uo_ipc_recv_first(
    void *recv_len,
    uo_cb *ipc_recv_cb)
{
    ssize_t len = (uintptr_t)recv_len;

    int wfd = (uintptr_t)uo_cb_stack_pop(ipc_recv_cb);
    uo_ipcmsg ipcmsg = uo_cb_stack_pop(ipc_recv_cb);
    uo_cb *ipcmsg_cb = uo_cb_stack_pop(ipc_recv_cb);

    if (len < sizeof(uint32_t))
    {
        free(ipcmsg);
        uo_cb_invoke_async(ipcmsg_cb, NULL, NULL);
        return NULL;
    }

    uint32_t payload_len = uo_ipcmsg_get_payload_len(ipcmsg);
    uint32_t total_len = payload_len + sizeof(uint32_t);

    if (total_len + 1 > UO_IPC_BUF_LEN)
        ipcmsg = realloc(ipcmsg, total_len + 1);

    ipcmsg[total_len] = '\0';

    uint32_t left_len = total_len - len;

    if (left_len)
    {
        char *p = ipcmsg + len;
        uo_cb *ipc_recvrest_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(ipc_recvrest_cb, uo_ipc_recv_rest);
        uo_cb_stack_push(ipc_recvrest_cb, ipcmsg_cb);
        uo_cb_stack_push(ipc_recvrest_cb, ipcmsg);
        uo_cb_stack_push(ipc_recvrest_cb, (void *)(uintptr_t)left_len);
        uo_cb_stack_push(ipc_recvrest_cb, p);
        uo_cb_stack_push(ipc_recvrest_cb, (void *)(uintptr_t)wfd);

        uo_io_read_async(wfd, p, left_len, ipc_recvrest_cb);
    }
    else
        uo_cb_invoke(ipcmsg_cb, ipcmsg);

    return NULL;
}

bool uo_ipc_sendmsg(
    int wfd,
    const char *src,
    size_t len,
    uo_cb *ipcmsg_cb)
{
    // write message synchronously
    if (!uo_ipcmsg_write(wfd, src, len))
        return false;

    // receive response asynchronously
    uo_ipcmsg ipcmsg = malloc(UO_IPC_BUF_LEN);

    uo_cb *ipc_recv_cb = uo_cb_create(UO_CB_OPT_DESTROY);
    uo_cb_append(ipc_recv_cb, uo_ipc_recv_first);
    uo_cb_stack_push(ipc_recv_cb, ipcmsg_cb);
    uo_cb_stack_push(ipc_recv_cb, ipcmsg);
    uo_cb_stack_push(ipc_recv_cb, (void *)(uintptr_t)wfd);

    return uo_io_read_async(wfd, ipcmsg, UO_IPC_BUF_LEN, ipc_recv_cb);
}
