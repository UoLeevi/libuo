#ifndef UO_IPCMSG_H
#define UO_IPCMSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef char *uo_ipcmsg;

uint32_t uo_ipcmsg_get_payload_len(
    uo_ipcmsg);

void uo_ipcmsg_set_payload_len(
    uo_ipcmsg,
    uint32_t payload_len);

char *uo_ipcmsg_get_payload(
    uo_ipcmsg);

bool uo_ipcmsg_write(
    int wfd,
    const char *src,
    uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
