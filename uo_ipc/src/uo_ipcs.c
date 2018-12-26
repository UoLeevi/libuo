#include "uo_ipcs.h"
#include "uo_tcp_server.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static void uo_ipcs_data_received(
    void *state, 
    uo_buf buf, 
    size_t new_data_len, 
    bool *recv_again)
{
    uint32_t msg_len;

    uo_buf_set_ptr_rel(buf, new_data_len);
    unsigned char *p = uo_buf_get_ptr(buf);

    if (p - buf >= sizeof (uint32_t))
        *recv_again = uo_ipcmsg_get_payload_len(buf) + sizeof (uint32_t) != p - buf;
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
    void *state, 
    uo_buf buf, 
    uo_cb *send_response_cb)
{
    uo_ipcs *ipcs = state;

    size_t payload_len = uo_ipcmsg_get_payload_len(buf);
    memmove(buf, uo_ipcmsg_get_payload(buf), payload_len);
    uo_buf_set_ptr_abs(buf, payload_len);

    uo_cb_prepend(send_response_cb, (void *(*)(void *, uo_cb *))uo_ipcs_buf_to_ipcmsg);

    ipcs->prepare_response(buf, send_response_cb);
}

uo_ipcs *uo_ipcs_create(
    char *servname,
    void (*prepare_response)(uo_buf, uo_cb *send_response_cb))
{
    uo_ipcs *ipcs = malloc(sizeof *ipcs);

    uo_tcp_server *tcp_server = uo_tcp_server_create(
        servname,
        ipcs,
        NULL,
        uo_ipcs_data_received,
        uo_ipcs_ipcmsg_to_buf);

    if (!tcp_server)
    {
        free(ipcs);
        return NULL;
    }
    
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
