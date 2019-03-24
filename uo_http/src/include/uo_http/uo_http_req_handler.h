#ifndef UO_HTTP_REQ_HANDLER_H
#define UO_HTTP_REQ_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_cb.h"
#include "uo_hashtbl.h"
#include "uo_refstack.h"
#include "uo_linklist.h"

#include <stddef.h>

typedef struct uo_http_req_handler
{
    uo_linklist link;
    char *req_pattern;
    size_t param_count;
    uo_cb *cb;
} uo_http_req_handler;

uo_http_req_handler *uo_http_req_handler_create(
    const char *req_pattern,
    const uo_cb *);

void uo_http_req_handler_destroy(
    uo_http_req_handler *);

bool uo_http_req_handler_try(
    uo_http_req_handler *,
    const char *method_sp_uri,
    uo_strhashtbl *params,
    uo_refstack *);

#ifdef __cplusplus
}
#endif

#endif
