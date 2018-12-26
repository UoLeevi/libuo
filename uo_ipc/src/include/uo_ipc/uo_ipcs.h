#ifndef UO_IPCS_H
#define UO_IPCS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_ipc.h"
#include "uo_buf.h"
#include "uo_cb.h"

typedef struct uo_ipcs
{
    void *tcp_server;
    void (*prepare_response)(uo_buf, uo_cb *uo_buf_cb);
} uo_ipcs;

uo_ipcs *uo_ipcs_create(
    char *servname,
    void (*prepare_response)(uo_buf, uo_cb *uo_buf_cb));

void uo_ipcs_destroy(
    uo_ipcs *ipcs);

#ifdef __cplusplus
}
#endif

#endif