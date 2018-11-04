#ifndef UO_IPCS_H
#define UO_IPCS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_ipc.h"
#include "uo_cb.h"

typedef struct uo_ipcs
{
    int sockfd;
    bool is_closing;
    void *listen_thrd;
    void *server_thrd;
    void *conn_queue;
    void *(*handle_msg)(uo_ipcmsg *, uo_cb *uo_ipcmsg_cb);
} uo_ipcs;

uo_ipcs *uo_ipcs_create(
    char *servname,
    size_t servname_len,
    void *(*handle_msg)(uo_ipcmsg *, uo_cb *uo_ipcmsg_cb));

#ifdef __cplusplus
}
#endif

#endif