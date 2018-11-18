#include "uo_ipcc.h"
#include "uo_err.h"
#include "uo_mem.h"
#include "uo_sock.h"

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <unistd.h>

#define RECV_TIMEOUT_SEC 10
#define SEND_TIMEOUT_SEC RECV_TIMEOUT_SEC

uo_ipcc *uo_ipcc_create(
    char *nodename,
    size_t nodename_len,
    char *servname,
    size_t servname_len)
{
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
    };

    uo_ipcc *ipcc = calloc(1, sizeof *ipcc);
    
    srand(time(NULL));
    ipcc->conn.eom = ~0 - (uint32_t)rand();

    int s = -1;

    uo_mem_using(nodename_ntbs, nodename_len + 1)
        uo_mem_using(servname_ntbs, servname_len + 1)
        {
            memcpy(nodename_ntbs, nodename, nodename_len);
            ((char *)nodename_ntbs)[nodename_len] = '\0';
            memcpy(servname_ntbs, servname, servname_len);
            ((char *)servname_ntbs)[servname_len] = '\0';

            s = getaddrinfo(nodename_ntbs, servname_ntbs, &hints, (struct addrinfo **)&ipcc->addrinfo);            
        }
    
    if (s != 0) 
		uo_err_goto(err_free, "Unable to create uo_ipcc. getaddrinfo: %s", gai_strerror(s));

    ipcc->conn.buf = malloc(ipcc->conn.buf_len = 0x100);

    return ipcc;

err_free:
    free(ipcc);

    return NULL;
}

void uo_ipcc_destroy(
    uo_ipcc *ipcc)
{
    if (ipcc->conn.sockfd)
        close(ipcc->conn.sockfd);

    freeaddrinfo(ipcc->addrinfo);
    free(ipcc->conn.buf);
    free(ipcc);
}

uo_ipcmsg uo_ipcc_send_msg(
    uo_ipcc *ipcc,
    uo_ipcmsg msg,
    bool is_last_msg)
{
    if (!ipcc->conn.sockfd)
    {
        struct addrinfo *addrinfo = ipcc->addrinfo;

        int sockfd = socket(addrinfo->ai_family, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd == -1)
            uo_err_return((uo_ipcmsg) { 0 }, "Unable to create socket.");

        ipcc->conn.sockfd = sockfd;

        int opt_TCP_NODELAY = true;
        if (uo_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY) == -1)
            uo_err("Could not set TCP_NODELAY option.");

        struct timeval opt_SO_RCVTIMEO = { .tv_sec = RECV_TIMEOUT_SEC };
        if (uo_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO) == -1)
            uo_err("Could not set SO_RCVTIMEO option.");

        struct timeval opt_SO_SNDTIMEO = { .tv_sec = SEND_TIMEOUT_SEC };
        if (uo_setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO) == -1)
            uo_err("Could not set SO_SNDTIMEO option.");

        if (connect(sockfd, addrinfo->ai_addr, addrinfo->ai_addrlen) == -1)
            uo_err_goto(err_close, "Unable to connect ipcc.");

        if (send(sockfd, (char *)&ipcc->conn.eom, sizeof ipcc->conn.eom, 0) == -1)
            uo_err_goto(err_close, "Error while sending ipc message.");
    }

    if (send(ipcc->conn.sockfd, msg.data, msg.data_len, 0) == -1)
        uo_err_goto(err_close, "Error while sending ipc message.");

    if (send(ipcc->conn.sockfd, (char *)&ipcc->conn.eom, sizeof ipcc->conn.eom, 0) == -1)
        uo_err_goto(err_close, "Error while sending ipc message.");

    if (is_last_msg)
        shutdown(ipcc->conn.sockfd, SHUT_WR);

    char *p = ipcc->conn.buf;
    size_t recv_buf_len = ipcc->conn.buf_len;
    ssize_t len;

    do
    {
        if ((len = recv(ipcc->conn.sockfd, p, recv_buf_len, 0)) == -1)
            uo_err_goto(err_close, "Error while receiving ipc message.");

        p += len;
        recv_buf_len -= len;

        if (!recv_buf_len)
        {
            recv_buf_len = ipcc->conn.buf_len;
            ptrdiff_t pdiff = p - ipcc->conn.buf;
            ipcc->conn.buf = realloc(ipcc->conn.buf, ipcc->conn.buf_len *= 2);
            p = ipcc->conn.buf + pdiff;
        }            
        *p = '\0';

    }  while (len && (*(uint32_t *)(p - sizeof ipcc->conn.eom) != ipcc->conn.eom));

    if (is_last_msg)
    {
        shutdown(ipcc->conn.sockfd, SHUT_RD);
        close(ipcc->conn.sockfd);
        ipcc->conn.sockfd = 0;
    }

    return (uo_ipcmsg) 
    {
        .data = ipcc->conn.buf,
        .data_len = (p - ipcc->conn.buf) - sizeof ipcc->conn.eom
    };

err_close:
    close(ipcc->conn.sockfd);
    ipcc->conn.sockfd = 0;

    return (uo_ipcmsg) { 0 };
}
