#ifndef UO_TCP_CLIENT_H
#define UO_TCP_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"

#include <stddef.h>
#include <stdint.h>

typedef struct uo_tcp_client
{
    /**
     * @brief evt_handlers struct contains callbacks for TCP client events
     * 
     * When TCP client raises an event it invokes a callback dedicated for that event.
     * The callback invoked has two pointers pushed on the stack of the callback:
     * a pointer to the TCP client instance and a pointer to the TCP connection instance.
     * These pointers can be accessed from within the event handler function like this:
     * 
     *  void uo_tcp_client_evt_handler(
     *      uo_cb *cb)
     *  {
     *      uo_tcp_client *tcp_client = uo_cb_stack_index(cb, 0);
     *      uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);
     * 
     *      // do stuff...
     * 
     *     uo_cb_invoke(cb);
     * }
     */
    struct
    {
        uo_cb *after_connect;
        uo_cb *before_send;
        uo_cb *after_send;
        uo_cb *before_recv;
        uo_cb *after_recv;
        uo_cb *after_close;
    } evt_handlers;
    struct
    {
        void *user_data;
    } conn_defaults;
    const char *hostname;
    const char *port;
} uo_tcp_client;

/**
 * @brief Create a TCP client
 * 
 * @param hostname          hostname to connect to
 * @param port              port to connect to
 * @return uo_tcp_client*   new TCP client instance
 */
uo_tcp_client *uo_tcp_client_create(
    const char *hostname,
    const char *port);

/**
 * @brief Create a TCP connection
 * 
 */
void uo_tcp_client_connect(
    uo_tcp_client *);

/**
 * @brief Free up the resources owned by the TCP client instance
 * 
 */
void uo_tcp_client_destroy(
    uo_tcp_client *);

#ifdef __cplusplus
}
#endif

#endif