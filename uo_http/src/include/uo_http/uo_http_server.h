#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_sess.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_http_server
{
    uo_http_sess_evt_handlers evt_handlers;
    struct
    {
        bool is_serving_static_files;
        union
        {
            const char *dirname;
        } param;
    } opt;
    struct
    {
        void *user_data;
    } sess_defaults;
    void *tcp_server;
} uo_http_server;

/**
 * @brief create a HTTP server
 * 
 * @param port              port to listen to for incoming connections
 * @return uo_http_server * pointer to a new HTTP server instance
 */
uo_http_server *uo_http_server_create(
    const char *port);

/**
 * @brief start the HTTP server
 * 
 */
void uo_http_server_start(
    uo_http_server *);

/**
 * @brief set option to serve static files from directory
 * 
 * @param dirname   a path to the base directory for the files to serve
 * @return true     on success
 * @return false    on error
 */
bool uo_http_server_set_opt_serve_static_files(
    uo_http_server *,
    const char *dirname);

/**
 * @brief free up the resources owned by the HTTP server instance
 * 
 */
void uo_http_server_destroy(
    uo_http_server *);

#ifdef __cplusplus
}
#endif

#endif