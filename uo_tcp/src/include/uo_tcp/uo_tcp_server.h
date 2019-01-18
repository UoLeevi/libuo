#ifndef UO_TCP_SERVER_H
#define UO_TCP_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct uo_tcp_server
{
    /**
     * @brief evt_handlers struct contains callbacks for TCP server events
     * 
     * When TCP server raises an event it invokes a callback dedicated for that event.
     * The callback invoked has two pointers pushed on the stack of the callback:
     * a pointer to the TCP server instance and a pointer to the TCP connection instance.
     * These pointers can be accessed from within the event handler function like this:
     * 
     *  void tcp_server_evt_handler(
     *      uo_cb *cb)
     *  {
     *      uo_tcp_server *tcp_server = uo_cb_stack_index(cb, 0);
     *      uo_tcp_conn *tcp_conn     = uo_cb_stack_index(cb, 1);
     * 
     *      // do stuff...
     * 
     *     uo_cb_invoke(cb);
     * }
     */
    struct
    {
        uo_cb *after_accept;
        uo_cb *before_recv;
        uo_cb *after_recv;
        uo_cb *before_send;
        uo_cb *after_send;
        uo_cb *after_close;
    } evt_handlers;
    struct
    {
        void *user_data;
    } conn_defaults;
    void *listen_thrd;
    void *server_thrd;
    void *conn_queue;
    int sockfd;
    bool is_closing;
} uo_tcp_server;

/**
 * @brief create a TCP server
 * 
 * @param port              port to listen to for incoming connections
 * @return uo_tcp_server*   pointer to a new TCP server instance
 */
uo_tcp_server *uo_tcp_server_create(
    const char *port);

/**
 * @brief start a TCP server
 * 
 */
void uo_tcp_server_start(
    uo_tcp_server *);

/**
 * @brief free up the resources owned by the TCP server instance
 * 
 */
void uo_tcp_server_destroy(
    uo_tcp_server *);

#ifdef __cplusplus
}
#endif

#endif
