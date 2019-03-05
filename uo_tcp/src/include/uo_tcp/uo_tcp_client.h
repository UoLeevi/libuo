#ifndef UO_TCP_CLIENT_H
#define UO_TCP_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_tcp_conn.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief TCP client
 * 
 * There are three steps to using the uo_tcp_client:
 *  1) create uo_tcp_client instance using uo_tcp_client_create
 *  2) register event handler functions for various TCP connection events
 *  3) make TCP connection(s) using uo_tcp_client_connect
 * 
 * @code
 * // create the uo_tcp_client instance
 * uo_tcp_client *tcp_client = uo_tcp_client_create("localhost", "12345");
 * 
 * // register event handlers
 * uo_cb_append(tcp_client->evt_handlers.after_open, my_tcp_client_evt_handler_after_open);
 * uo_cb_append(tcp_client->evt_handlers.before_send, my_tcp_client_evt_handler_before_send);
 * uo_cb_append(tcp_client->evt_handlers.after_recv, my_tcp_client_evt_handler_after_recv);
 * 
 * // open a new TCP connection
 * uo_tcp_client_connect(tcp_client);
 * @endcode
 */
typedef struct uo_tcp_client
{
    uo_strhashtbl *user_data;
    uo_tcp_conn_evt_handlers evt_handlers;
    const char *hostname;
    const char *port;
} uo_tcp_client;

/**
 * @brief create a TCP client
 * 
 * @param hostname          hostname to connect to
 * @param port              port to connect to
 * @return uo_tcp_client*   new TCP client instance
 */
uo_tcp_client *uo_tcp_client_create(
    const char *hostname,
    const char *port);

/**
 * @brief get a pointer to the user data that has been set using uo_tcp_client_set_user_data
 * 
 * @param key       null terminated string key
 * @return void *   a pointer to the user data
 */
void *uo_tcp_client_get_user_data(
    uo_tcp_client *,
    const char *key);

/**
 * @brief store an arbitrary pointer that can be later accessed using uo_tcp_client_get_user_data
 * 
 * @param key           null terminated string key
 * @param user_data     a pointer to arbitrary user data
 */
void uo_tcp_client_set_user_data(
    uo_tcp_client *,
    const char *key,
    void *user_data);

bool uo_tcp_client_set_opt_use_flow_send_recv_repeat(
    uo_tcp_client *);

bool uo_tcp_client_set_opt_use_flow_send_recv_close(
    uo_tcp_client *);

/**
 * @brief use length prefixed messages
 * 
 * @return true     on success
 * @return false    on error
 */
bool uo_tcp_client_set_opt_use_length_prefixed_messages(
    uo_tcp_client *);

/**
 * @brief open a new TCP connection
 * 
 */
void uo_tcp_client_connect(
    uo_tcp_client *);

/**
 * @brief free up the resources owned by the TCP client instance
 * 
 */
void uo_tcp_client_destroy(
    uo_tcp_client *);

#ifdef __cplusplus
}
#endif

#endif