#ifndef UO_IPC_H
#define UO_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_ipcconn
{
    int sockfd;
    char eom[4];
    char *buf;
    size_t buf_len;
} uo_ipcconn;

typedef struct uo_ipcmsg
{
    char *data;
    size_t data_len;
    bool should_free;
} uo_ipcmsg;

bool uo_ipc_init(void);

#ifdef __cplusplus
}
#endif

#endif