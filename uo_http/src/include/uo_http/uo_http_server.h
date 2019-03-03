#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_sess.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_tcp_server uo_tcp_server;

/**
 * @brief HTTP server
 * 
 * uo_http_server is an event-driven HTTP server that support asynchronous.
 * 
 * uo_http_server is used in three steps:
 *  1. create uo_http_server instance calling uo_http_server_create
 *  2. set up options and handlers using uo_http_server_set_opt_* and 
 *     uo_http_server_add_request_handler[_*] functions
 *  3. start the uo_http_server by calling uo_http_start
 * 
 * Additionally you can directly add callbacks to various HTTP session 
 * lifecycle events with uo_cb_append and the evt_handlers struct. 
 *  E.g. 
 *    uo_cb_append(
 *        http_server->evt_handlers.before_send_msg, // subscribed event
 *        http_server_before_send_response);         // a function with signature: void (*)(uo_cb *)
 * 
 * The HTTP lifecycle events are as follows:
 *   after_open
 *    - after new TCP connection has been accepted 
 *   after_recv_headers
 *    - after request headers have been received and parsed
 *   after_recv_msg
 *    - after request has been fully received and parsed
 *   before_send_msg
 *    - after all request handlers have been processed and response is ready to be sent
 *   after_send_msg
 *    - after response is sent back to the client
 *   before_close
 *    - before server closes the TCP socket
 *   after_close
 *    - after server has closed the TCP socket
 */
typedef struct uo_http_server
{
    uo_http_sess_evt_handlers evt_handlers;
    struct
    {
        uo_strhashtbl *GET;
        uo_strhashtbl *POST;
        uo_strhashtbl *PUT;
        uo_strhashtbl *DELETE;
    } request_handlers;
    struct
    {
        bool is_serving_static_files;
        struct
        {
            const char *dirname;
        } param;
    } opt;
    struct
    {
        void *user_data;
    } sess_defaults;
    uo_tcp_server *tcp_server;
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
 * @brief add a handler (i.e. callback) for specific request uri and method
 * 
 * Only single handler for each URI-method pair is permitted.
 * 
 * @param method    request method
 * @param uri       request URI
 * @param handler   handler callback
 * @return true     on success
 * @return false    on error
 */
bool uo_http_server_add_request_handler(
    uo_http_server *,
    uo_http_method method,
    const char *uri,
    uo_cb *handler);

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