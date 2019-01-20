#ifndef UO_TCP_SERVER_H
#define UO_TCP_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief TCP server
 * 
 * There are three steps to using the uo_tcp_server:
 *  1) create uo_tcp_server instance using uo_tcp_server_create
 *  2) register event handler functions for various TCP connection events
 *  3) start the TCP server using uo_tcp_server_start
 * 
 * @code
 * // create the uo_tcp_server instance
 * uo_tcp_server *tcp_server = uo_tcp_server_create("12345");
 * 
 * // register event handlers
 * uo_cb_append(tcp_server->evt_handlers.after_open, my_tcp_server_evt_handler_after_open);
 * uo_cb_append(tcp_server->evt_handlers.after_recv, my_tcp_server_evt_handler_after_recv);
 * uo_cb_append(tcp_server->evt_handlers.before_send, my_tcp_server_evt_handler_before_send);
 * 
 * // start the uo_tcp_server
 * uo_tcp_server_start(tcp_server);
 * @endcode
 */
typedef struct uo_tcp_server
{
    uo_tcp_conn_evt_handlers evt_handlers;
    struct
    {
        void *user_data;
    } conn_defaults;
    void *thrd;
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
