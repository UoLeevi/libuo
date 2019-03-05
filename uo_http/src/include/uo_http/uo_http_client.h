#ifndef UO_HTTP_CLIENT_H
#define UO_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_conn.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_tcp_client uo_tcp_client;
typedef struct uo_strhashtbl uo_strhashtbl;

typedef struct uo_http_client
{
    uo_strhashtbl *user_data;
    uo_http_conn_evt_handlers evt_handlers;
    uo_tcp_client *tcp_client;
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
 * @brief get a pointer to the user data that has been set using uo_http_client_set_user_data
 * 
 * @param key       null terminated string key
 * @return void *   a pointer to the user data
 */
void *uo_http_client_get_user_data(
    uo_http_client *,
    const char *key);

/**
 * @brief store an arbitrary pointer that can be later accessed using uo_http_client_get_user_data
 * 
 * @param key           null terminated string key
 * @param user_data     a pointer to arbitrary user data
 */
void uo_http_client_set_user_data(
    uo_http_client *,
    const char *key,
    void *user_data);

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