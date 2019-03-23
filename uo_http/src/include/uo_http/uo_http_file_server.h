#ifndef UO_HTTP_FILE_SERVER_H
#define UO_HTTP_FILE_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_hashtbl.h"

#include <stddef.h>

typedef struct uo_http_file_server
{
    uo_strhashtbl cache;
    const char *dirname;
    size_t cache_space;
    void *mtx;
} uo_http_file_server;

uo_http_file_server *uo_http_file_server_create(
    const char *dirname,
    size_t cache_size);

void uo_http_file_server_destroy(
    uo_http_file_server *);

void uo_http_file_server_serve(
    uo_http_file_server *,
    uo_http_conn *);

#ifdef __cplusplus
}
#endif

#endif