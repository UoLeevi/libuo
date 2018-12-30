#ifndef UO_CONF_H
#define UO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct uo_conf {
	void *hashtbl;
} uo_conf;

uo_conf *uo_conf_create(
	char *filepath);

char *uo_conf_get(
	uo_conf *,
	const char *key);

void uo_conf_destroy(
	uo_conf *);

#ifdef __cplusplus
}
#endif

#endif