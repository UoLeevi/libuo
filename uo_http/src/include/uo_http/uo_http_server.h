#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct uo_http_server
{
    void *tcp_server;
    char *root_dir;
} uo_http_server;

uo_http_server *uo_http_server_create(
    const char *port);

void uo_http_server_start(
    uo_http_server *);

bool uo_http_server_set_root_dir(
    uo_http_server *,
    const char *root_dir);

void uo_http_server_destroy(
    uo_http_server *);

#ifdef __cplusplus
}
#endif

#endif