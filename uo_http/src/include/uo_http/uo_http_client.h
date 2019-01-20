#ifndef UO_HTTP_CLIENT_H
#define UO_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_http_client
{
    /**
     * @brief evt_handlers struct contains callbacks for HTTP client events
     * 
     * When HTTP client raises an event it invokes a callback dedicated for that event.
     * The callback invoked has four pointers pushed on the stack of the callback:
     * two pointers related to underlying TCP client event that are used internally,
     * a pointer to the HTTP client instance and a pointer to the HTTP connection instance.
     * These pointers can be accessed from within the event handler function like this:
     * 
     *  void http_client_evt_handler(
     *      uo_cb *cb)
     *  {
     *      uo_http_client *http_client = uo_cb_stack_index(cb, 1);
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
        uo_cb *before_send_request;
        uo_cb *after_send_request;
        uo_cb *after_recv_headers;
        uo_cb *after_recv_response;
        uo_cb *after_close;
    } evt_handlers;
    struct
    {

    } opt;
    struct
    {
        void *user_data;
    } conn_defaults;
    void *tcp_client;
} uo_http_client;

/**
 * @brief create a HTTP client
 * 
 * @param hostname          hostname to connect to
 * @param port              port to connect to
 * @return uo_http_client*  new HTTP client instance
 */
uo_http_client *uo_http_client_create(
    const char *hostname,
    const char *port);

/**
 * @brief create a TCP connection
 * 
 */
void uo_http_client_connect(
    uo_http_client *);

/**
 * @brief free up the resources owned by the HTTP client instance
 * 
 */
void uo_http_client_destroy(
    uo_http_client *);

#ifdef __cplusplus
}
#endif

#endif