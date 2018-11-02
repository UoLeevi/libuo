#ifndef UO_TCPSERV_CONF_H
#define UO_TCPSERV_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define UO_TCPSERV_CONF_FILE "uo_tcpserv.conf"

typedef struct uo_tcpserv_key {
	char *name;
	size_t name_len;
    char *key;
    size_t key_len;
} uo_tcpserv_key;

typedef struct uo_tcpserv_conf {
	uo_tcpserv_key *keys;
    size_t keys_len;
	uint16_t port;
} uo_tcpserv_conf;

uo_tcpserv_conf *uo_tcpserv_conf_create(void);

void uo_tcpserv_conf_destroy(
	uo_tcpserv_conf *);

#ifdef __cplusplus
}
#endif

#endif