#ifndef UO_CB_H
#define UO_CB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

#include <semaphore.h>

typedef enum UO_CB_OPT
{
	UO_CB_OPT_DESTROY = 	1 << 0
} UO_CB_OPT;

typedef struct uo_cb 
{
	size_t count;
	void *(**f)(void *arg, struct uo_cb *);
	size_t stack_top;
	void **stack;
	UO_CB_OPT opt;
} uo_cb;

bool uo_cb_init(void);

uo_cb *uo_cb_create(
	UO_CB_OPT);

void uo_cb_destroy(
	uo_cb *);

void uo_cb_append(
	uo_cb *,
	void *(*)(void *arg, uo_cb *));

void uo_cb_prepend(
	uo_cb *,
	void *(*)(void *arg, uo_cb *));

void *uo_cb_invoke(
	uo_cb *cb,
	void *arg);

void uo_cb_invoke_async(
	uo_cb *cb,
	void *arg,
	sem_t *sem);

void uo_cb_stack_push(
	uo_cb *, 
	void *);

void *uo_cb_stack_pop(
	uo_cb *);

void *uo_cb_stack_peek(
	uo_cb *);

#ifdef __cplusplus
}
#endif

#endif