#ifndef UO_HTTP_SERVER_H
#define UO_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_tcp_server uo_tcp_server;
typedef struct uo_strhashtbl uo_strhashtbl;

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
    uo_strhashtbl user_data;
    uo_http_conn_evt_handlers evt_handlers;
    struct
    {
        // handlers matched using request line without version. 
        // E.g.
        //   "GET /asdf"
        //   "POST /user"
        uo_strhashtbl prefix;
        uo_strhashtbl exact;
    } req_handlers;
    struct
    {
        bool is_serving_static_files;
        struct
        {
            const char *dirname;
        } param;
    } opt;
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
 * @brief get a pointer to the user data that has been set using uo_http_server_set_user_data
 * 
 * @param key       null terminated string key
 * @return void *   a pointer to the user data
 */
void *uo_http_server_get_user_data(
    uo_http_server *,
    const char *key);

/**
 * @brief store an arbitrary pointer that can be later accessed using uo_http_server_get_user_data
 * 
 * @param key           null terminated string key
 * @param user_data     a pointer to arbitrary user data
 */
void uo_http_server_set_user_data(
    uo_http_server *,
    const char *key,
    const void *user_data);

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
 * @brief add or update a prefix handler (i.e. callback) for specific request line
 * 
 * Handlers are processed until response status line has been set.
 * Request handler matching process is as follows:
 *  1. Check for prexix match for each path segment in request line
 *  2. Check for exact match
 * 
 * @param method_sp_uri     null terminated string containing HTTP method and partial request URI
 * @param handler           handler callback
 * @return true             on success
 * @return false            on error
 */
#define uo_http_server_set_req_prefix_handler(http_server, method_sp_uri, handler) _Generic((handler), \
    uo_cb_func: uo__http_server_set_req_prefix_func_handler, \
       uo_cb *: uo__http_server_set_req_prefix_cb_handler)(http_server, method_sp_uri, handler)

bool uo__http_server_set_req_prefix_func_handler(
    uo_http_server *,
    const char *method_sp_uri,
    uo_cb_func handler);

bool uo__http_server_set_req_prefix_cb_handler(
    uo_http_server *,
    const char *method_sp_uri,
    uo_cb *handler);

/**
 * @brief add or update an exact handler (i.e. callback) for specific request line
 * 
 * Handlers are processed until response status line has been set.
 * Request handler matching process is as follows:
 *  1. Check for prexix match for each path segment in request line
 *  2. Check for exact match
 * 
 * @param method_sp_uri     null terminated string containing HTTP method and request URI
 * @param handler           handler callback
 * @return true             on success
 * @return false            on error
 */
#define uo_http_server_set_req_exact_handler(http_server, method_sp_uri, handler) _Generic((handler), \
    uo_cb_func: uo__http_server_set_req_exact_func_handler, \
       uo_cb *: uo__http_server_set_req_exact_cb_handler)(http_server, method_sp_uri, handler)

bool uo__http_server_set_req_exact_func_handler(
    uo_http_server *,
    const char *method_sp_uri,
    uo_cb_func handler);

bool uo__http_server_set_req_exact_cb_handler(
    uo_http_server *,
    const char *method_sp_uri,
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