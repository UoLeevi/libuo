#include "uo_ipcs.h"
#include "uo_tcp_server.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static void uo_ipcs_after_recv(
    uo_tcp_conn *tcp_conn,
    uo_cb *cb)
{
    size_t msg_len = uo_buf_get_len_before_ptr(tcp_conn->buf);

    tcp_conn->evt_data.recv_again = msg_len >= sizeof (uint32_t) 
        && uo_ipcmsg_get_payload_len(tcp_conn->buf) + sizeof (uint32_t) != msg_len;

    uo_cb_invoke(cb, NULL);
}

static uo_ipcmsg uo_ipcs_buf_to_ipcmsg(
    uo_buf buf,
    uo_cb *send_response_cb)
{
    if (uo_buf_get_len_after_ptr(buf) < sizeof (uint32_t))
        buf = uo_buf_realloc(buf, uo_buf_get_size(buf) + sizeof (uint32_t));

    size_t payload_len = uo_buf_get_len_before_ptr(buf);
    memmove(uo_ipcmsg_get_payload(buf), buf, payload_len);
    uo_ipcmsg_set_payload_len(buf, payload_len);
    uo_buf_set_ptr_rel(buf, sizeof (uint32_t));

    return buf;
}

void uo_ipcs_ipcmsg_to_buf(
    uo_tcp_conn *tcp_conn, 
    uo_cb *cb)
{
    uo_ipcs *ipcs = tcp_conn->state;
    uo_buf buf = tcp_conn->buf;

    size_t payload_len = uo_ipcmsg_get_payload_len(buf);
    memmove(buf, uo_ipcmsg_get_payload(buf), payload_len);
    uo_buf_set_ptr_abs(buf, payload_len);

    uo_cb_prepend(cb, (void *(*)(void *, uo_cb *))uo_ipcs_buf_to_ipcmsg);

    ipcs->prepare_response(buf, cb);
}

uo_ipcs *uo_ipcs_create(
    char *servname,
    void (*prepare_response)(uo_buf, uo_cb *send_response_cb))
{
    uo_ipcs *ipcs = malloc(sizeof *ipcs);

    uo_tcp_server *tcp_server = uo_tcp_server_create(servname, ipcs);
    if (!tcp_server)
    {
        free(ipcs);
        return NULL;
    }

    tcp_server->evt.after_recv_handler = uo_ipcs_after_recv;
    tcp_server->evt.before_send_handler = uo_ipcs_ipcmsg_to_buf;
    uo_tcp_server_start(tcp_server);
    
    ipcs->tcp_server = tcp_server;
    ipcs->prepare_response = prepare_response;	

    return ipcs;
}

void uo_ipcs_destroy(
    uo_ipcs *ipcs)
{
    uo_tcp_server_destroy(ipcs->tcp_server);
    free(ipcs);
}
