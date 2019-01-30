#ifndef UO_HTTP_CLIENT_H
#define UO_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_http_sess.h"
#include "uo_cb.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct uo_http_client
{
    uo_http_sess_evt_handlers evt_handlers;
    struct
    {

    } opt;
    struct
    {
        void *user_data;
    } sess_defaults;
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