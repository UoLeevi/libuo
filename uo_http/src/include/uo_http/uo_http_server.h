#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_http_server
{
    /**
     * @brief evt_handlers struct contains callbacks for HTTP server events
     * 
     * When HTTP server raises an event it invokes a callback dedicated for that event.
     * The callback invoked has four pointers pushed on the stack of the callback:
     * two pointers related to underlying TCP server event that are used internally,
     * a pointer to the HTTP server instance and a pointer to the HTTP connection instance.
     * These pointers can be accessed from within the event handler function like this:
     * 
     *  void http_server_evt_handler(
     *      uo_cb *cb)
     *  {
     *      uo_http_server *http_server = uo_cb_stack_index(cb, 1);
     *      uo_http_conn *http_conn     = uo_cb_stack_index(cb, 2);
     * 
     *      // do stuff...
     * 
     *     uo_cb_invoke(cb);
     * }
     */
    struct
    {
        uo_cb *after_open;
        uo_cb *after_recv_headers;
        uo_cb *after_recv_request;
        uo_cb *before_send_response;
        uo_cb *after_send_response;
        uo_cb *after_close;
    } evt_handlers;
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
    } conn_defaults;
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