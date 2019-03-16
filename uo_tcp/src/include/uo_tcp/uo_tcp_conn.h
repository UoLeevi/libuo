#ifndef UO_TCP_CONN_H
#define UO_TCP_CONN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "uo_hashtbl.h"
#include "uo_cb.h"
#include "uo_buf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

typedef struct uo_tcp_server uo_tcp_server;
typedef struct uo_tcp_client uo_tcp_client;

/**
 * @brief uo_tcp_conn_evt_handlers struct contains callbacks for TCP connection events
 * 
 * When TCP connection raises an event it invokes a callback dedicated for that event.
 * The callback invoked has a pointer to the TCP connection instance pushed on the stack of the callback.
 * Do not remove this pointer form the callback stack.
 * The pointer to TCP connection instance can be accessed from within the event handler function like this:
 * 
 *  void tcp_conn_evt_handler(
 *      uo_cb *cb)
 *  {
 *      uo_tcp_conn *tcp_conn = uo_cb_stack_index(cb, 0);
 * 
 *      // do stuff...
 * 
 *     uo_cb_invoke(cb);
 * }
 */
typedef struct uo_tcp_conn_evt_handlers
{
    uo_cb *after_open;
    uo_cb *before_recv;
    uo_cb *after_recv;
    uo_cb *before_send;
    uo_cb *after_send;
    uo_cb *before_close;
    uo_cb *after_close;
} uo_tcp_conn_evt_handlers;

typedef union uo_tcp_conn_evt_arg
{
    struct
    {
        size_t last_recv_len;
        bool is_msg_fully_received;
    } after_recv;
} uo_tcp_conn_evt_arg;

typedef struct uo_tcp_conn
{
    uo_strhashtbl user_data;
    uo_strhashtbl *shared_user_data;
    uo_buf rbuf;
    uo_buf wbuf;
    uo_tcp_conn_evt_handlers *evt_handlers;
    uo_tcp_conn_evt_arg evt_arg;
    int sockfd;
    /**
     * @brief code for TCP connection state
     * 
     * Possible values are:
     * '\0': no pending operation
     * 'r':  pending recv
     * 's':  pending send
     * 'c':  pending close
     * 'i':  idle
     */
    atomic_char state;
} uo_tcp_conn;

/**
 * @brief get a pointer to the user data that has been set using uo_tcp_conn_set_user_data
 * 
 * If there is no user_data set for the tcp_conn then user_data would be checked for
 * the uo_tcp_server or uo_tcp_client that opened the connection.
 * 
 * @param key       null terminated string key
 * @return void *   a pointer to the user data
 */
void *uo_tcp_conn_get_user_data(
    uo_tcp_conn *,
    const char *key);

/**
 * @brief store an arbitrary pointer that can be later accessed using uo_tcp_conn_set_user_data
 * 
 * The stored user_data is specific to the connection that is specified as the first argument.
 * 
 * @param key           null terminated string key
 * @param user_data     a pointer to arbitrary user data
 */
void uo_tcp_conn_set_user_data(
    uo_tcp_conn *,
    const char *key,
    const void *user_data);

/**
 * @brief instruct the TCP connection to read data from the socket and append it to rbuf
 * 
 * This is one of three functions which can be used to control the flow of the communication over the TCP connection
 */
void uo_tcp_conn_next_recv(
    uo_tcp_conn *);

/**
 * @brief instruct the TCP connection to write the data in the wbuf to the socket
 * 
 * This is one of three functions which can be used to control the flow of the communication over the TCP connection
 */
void uo_tcp_conn_next_send(
    uo_tcp_conn *);

/**
 * @brief instruct the TCP connection to close the socket
 * 
 * This is one of three functions which can be used to control the flow of the communication over the TCP connection
 */
void uo_tcp_conn_next_close(
    uo_tcp_conn *);

#ifdef __cplusplus
}
#endif

#endif
