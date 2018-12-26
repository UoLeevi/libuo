#ifndef UO_IPC_H
#define UO_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_ipcmsg.h"
#include "uo_cb.h"

#include <stdbool.h>

bool uo_ipc_init(void);

int uo_ipc_connect(
    char *nodename,
    char *servname);

void uo_ipc_disconnect(
    int fd);

bool uo_ipc_sendmsg(
    int wfd,
    const char *src,
    size_t len,
    uo_cb *ipcmsg_cb);

#ifdef __cplusplus
}
#endif

#endif
