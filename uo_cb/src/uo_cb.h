#ifndef UO_CB_H
#define UO_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <semaphore.h>

typedef enum uo_cb_opt {
	uo_cb_opt_invoke_once = 1 << 0
} uo_cb_opt;

typedef struct uo_cb {
	size_t count;
	void *(**f)(void *arg, void *state);
	uo_cb_opt opt;
} uo_cb;

bool uo_cb_init(
	size_t thrd_count);

uo_cb *uo_cb_create(
	uo_cb_opt);

void uo_cb_destroy(
	uo_cb *);

void uo_cb_append(
	uo_cb *,
	void *(*)(void *arg, void *state));

void *uo_cb_invoke(
	uo_cb *cb,
	void *arg,
	void *state);

void uo_cb_invoke_async(
	uo_cb *cb,
	void *arg,
	void *state,
	sem_t *sem);

void *(*uo_cb_as_func(
	uo_cb *, 
	void *state))(void *arg, void *state);

#ifdef __cplusplus
}
#endif

#endif