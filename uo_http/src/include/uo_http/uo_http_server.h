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
    uo_linklist req_handlers;
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
 * @brief add a request handler (i.e. callback) for specific request line that matches a pattern
 * 
 * Mathcing handlers are processed until response status line has been set.
 * Request patterns are of form <HTTP method> <URI with patter parameters>[*]
 * E.g.
 *  GET /
 *  GET /about
 *  GET /posts/{title}/
 *  POST /user/*
 *  POST /notes/{note_uuid}/{tag_uuid}/
 *  PUT /{year}-{month}-{day}/complex-endpoint/*
 * 
 * @param req_pattern     null terminated string containing HTTP method and request URI 
 *                        with possible pattern variables and optional ending asterix
 * @param handler         handler callback
 * @return true           on success
 * @return false          on error
 */
#define uo_http_server_add_req_handler(http_server, req_pattern, handler) _Generic((handler), \
    uo_cb_func: uo__http_server_add_req_func_handler, \
       uo_cb *: uo__http_server_add_req_cb_handler)(http_server, req_pattern, handler)

void uo__http_server_add_req_func_handler(
    uo_http_server *,
    const char *req_pattern,
    uo_cb_func handler);

void uo__http_server_add_req_cb_handler(
    uo_http_server *,
    const char *req_pattern,
    const uo_cb *handler);

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
