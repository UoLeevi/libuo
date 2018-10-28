#include "uo_tcpserv.h"
#include "uo_tcpserv_conf.h"
#include "uo_queue.h"
#include "uo_sock.h"
#include "uo_cb.h"
#include "uo_err.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

static int server_sockfd;
static bool is_closing;
static uo_queue *conn_queue;

static void uo_tcpserv_handle_signal(
    int sig) 
{
	is_closing = true;
	close(server_sockfd);
	signal(sig, SIG_DFL);
}

static void *uo_tcpserv_send_res(
    uo_tcpserv_res *res, 
    uo_cb *uo_tcpserv_res_cb)
{
    int *sockfd_ptr = uo_cb_pop_data(uo_tcpserv_res_cb);
    int client_sockfd = *sockfd_ptr;
    free(sockfd_ptr);

    if (res && res->data)
    {
        send(client_sockfd, res->data, res->data_len, 0);
        free(res->data);
    }

    shutdown(client_sockfd, SHUT_WR);
    close(client_sockfd);
    free(res);

    return NULL;
}

static void *uo_tcpserv_serve(
    void *arg) 
{
    void *(*handle_cmd)(uo_tcpserv_arg *, uo_cb *uo_tcpserv_res_cb) = arg;

    size_t buf_len = 0x1000;
    char *buf = malloc(buf_len);
    while (!is_closing)
    {
        int *client_sockfd = uo_queue_dequeue(conn_queue, true);
            
        ssize_t recv_len = 0;
        char *p = buf;
        
        do
        {
            if ((recv_len = recv(*client_sockfd, p, buf_len - (p - buf), 0)) == -1)
                uo_err_goto(err_close, "Error while receiving the command message.");

            p += recv_len;

            if (p - buf == buf_len)
            {
                ptrdiff_t pdiff = buf_len;
                buf = realloc(buf, buf_len *= 2);
                p = buf + pdiff;
            }

        } while (recv_len);

        shutdown(*client_sockfd, SHUT_RD);

        size_t data_len = p - buf;
        char *data = malloc(data_len + 1);
        memcpy(data, buf, data_len);
        data[data_len] = '\0';

        uo_tcpserv_arg *cmd = malloc(sizeof *cmd);
        cmd->data = data;
        cmd->data_len = data_len;

        uo_cb *uo_tcpserv_res_cb = uo_cb_create(UO_CB_OPT_DESTROY);
        uo_cb_append(uo_tcpserv_res_cb, (void *(*)(void *, uo_cb *))uo_tcpserv_send_res);
        uo_cb_push_data(uo_tcpserv_res_cb, client_sockfd);

        handle_cmd(cmd, uo_tcpserv_res_cb);

        continue;

err_close:
        close(*client_sockfd);
    }
    
    return NULL;
}

void uo_tcpserv_start(
    bool (*configure_cmd_handler)(uo_tcpserv_arg),
    void *(*handle_cmd)(uo_tcpserv_arg *, uo_cb *uo_tcpserv_res_cb))
{
    uo_tcpserv_conf *conf = uo_tcpserv_conf_create();
    conn_queue = uo_queue_create(0x100);

    if (!uo_sock_init())
        uo_err_exit("Error initializing uo_sock.");

    if (!uo_cb_init(2))
        uo_err_exit("Error initializing uo_cb.");

    if (!configure_cmd_handler((uo_tcpserv_arg) { .data = conf->keys[0].key, .data_len = conf->keys[0].key_len }))
        uo_err_exit("Unable to configure client.");

    pthread_t thrd;
    pthread_create(&thrd, NULL, uo_tcpserv_serve, handle_cmd);

    /*	Start listening socket that will accept and queue new connections

        The socket is blocking dual-stack socket that will listen port that was set in configuration file. */
	server_sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (server_sockfd == -1)
		uo_err_exit("Unable to create socket");
	
	{
		int opt_IPV6_V6ONLY = false;
		uo_setsockopt(server_sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_IPV6_V6ONLY, sizeof opt_IPV6_V6ONLY);

		int opt_TCP_NODELAY = true;
		uo_setsockopt(server_sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_TCP_NODELAY, sizeof opt_TCP_NODELAY);

        struct timeval opt_SO_RCVTIMEO = { .tv_sec = 20 };
		uo_setsockopt(server_sockfd, IPPROTO_IPV6, SO_RCVTIMEO, &opt_SO_RCVTIMEO, sizeof opt_SO_RCVTIMEO);

        struct timeval opt_SO_SNDTIMEO = { .tv_sec = 20 };
        uo_setsockopt(server_sockfd, IPPROTO_IPV6, SO_SNDTIMEO, &opt_SO_SNDTIMEO, sizeof opt_SO_SNDTIMEO);
    }

    struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = conf->port,
        .sin6_addr = in6addr_any
    };

    if (bind(server_sockfd, (struct sockaddr *)&addr, sizeof addr) == -1)
        uo_err_exit("Unable to bind to socket!");

    /*	Setup signal handling
        
        TODO: Consider using sigaction instead of signal */
    signal(SIGINT, uo_tcpserv_handle_signal);

    if (listen(server_sockfd, SOMAXCONN) == -1)
        uo_err_exit("Unable to listen on socket!\r\n");

    {
        char addrp[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.sin6_addr, addrp, INET6_ADDRSTRLEN);
        uint16_t portp = ntohs(addr.sin6_port);

        printf("Listening on [%s]:%u.\r\n", addrp, portp);
    }

    while (!is_closing) 
    {
        /*	Accept connection

            Client address can be later acquired with 
            int getsockname(int sockfd, struct sockaddr *addrsocklen_t *" addrlen ); */
        int *client_sockfd = malloc(sizeof *client_sockfd);
        if ((*client_sockfd = accept(server_sockfd, NULL, NULL)) != -1)
            uo_queue_enqueue(conn_queue, client_sockfd, true);
        else
        {
            uo_err("Error on accepting connection.");
            free(client_sockfd);
        }
    }

    pthread_join(thrd, NULL);

    uo_queue_destroy(conn_queue);
}