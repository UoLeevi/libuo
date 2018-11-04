#ifndef UO_IPCC_H
#define UO_IPCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_ipc.h"

#include <stddef.h>

typedef struct uo_ipcc
{
    void *addrinfo;
    uo_ipcconn conn;
} uo_ipcc;

uo_ipcc *uo_ipcc_create(
    char *nodename,
    size_t nodename_len,
    char *servname,
    size_t servname_len);

void uo_ipcc_destroy(
    uo_ipcc *);

uo_ipcmsg uo_ipcc_send_msg(
    uo_ipcc *,
    uo_ipcmsg,
    bool is_last_msg);

#ifdef __cplusplus
}
#endif

#endif