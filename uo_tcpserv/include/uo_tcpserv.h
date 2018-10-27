#ifndef UO_TCPSERV_H
#define UO_TCPSERV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stddef.h"

typedef struct uo_tcpserv_data
{
    char *data;
    size_t data_len;
} uo_tcpserv_arg, uo_tcpserv_res;

void uo_tcpserv_start(
    bool (*configure_cmd_handler)(uo_tcpserv_arg),
    void *(*handle_cmd)(uo_tcpserv_arg *, void *state, void *(*send_res)(uo_tcpserv_res *, void *state)));

#ifdef __cplusplus
}
#endif

#endif