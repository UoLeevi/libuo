#include "uo_cb.h"
#include "uo_cb_queue.h"
#include "uo_cb_thrd_pool.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdatomic.h>

#define UO_CB_F_MIN_ALLOC 2
#define UO_CB_STACK_MIN_ALLOC 2

static bool is_init;

bool uo_cb_init() 
{
	if (is_init)
		return true;

	is_init = true;

	is_init &= uo_cb_queue_init();
	is_init &= uo_cb_thrd_pool_init();

	return is_init;
}

uo_cb *uo_cb_create(
	UO_CB_OPT opt)
{
	uo_cb *cb = calloc(1, sizeof *cb);
	cb->f = malloc(sizeof *cb->f * UO_CB_F_MIN_ALLOC);
	cb->opt = opt;
	return cb;
}

void uo_cb_destroy(
	uo_cb *cb)
{
	free(cb->f);
	free(cb->stack);
	free(cb);
}

void uo_cb_append(
	uo_cb *cb,
	void *(*f)(void *arg, uo_cb *))
{
	size_t count = cb->count++;

	if (count >= UO_CB_F_MIN_ALLOC && !(count & (count - 1)))
		cb->f = realloc(cb->f, sizeof *cb->f * count << 1);

	cb->f[count] = f;
}

void uo_cb_prepend(
	uo_cb *cb,
	void *(*f)(void *arg, uo_cb *))
{
	size_t count = cb->count++;

	if (count >= UO_CB_F_MIN_ALLOC && !(count & (count - 1)))
		cb->f = realloc(cb->f, sizeof *cb->f * count << 1);
	
	memmove(cb->f + 1, cb->f, sizeof *cb->f * count);
	*cb->f = f;
}

void *uo_cb_invoke(
	uo_cb *cb,
	void *arg)
{
	void *result = arg;

	for (size_t i = 0; i < cb->count; ++i)
		result = cb->f[i](result, cb);

	if (cb->opt & UO_CB_OPT_DESTROY)
		uo_cb_destroy(cb);

	return result;
}

void uo_cb_invoke_async(
	uo_cb *cb,
	void *arg,
	sem_t *sem)
{
	if (sem) 
		sem_init(sem, 0, 0);
	
	uo_cb_stack_push(cb, arg);
	uo_cb_stack_push(cb, sem);

	uo_cb_queue_enqueue(cb);
}

void uo_cb_stack_push(
	uo_cb *cb, 
	void *ptr)
{
	size_t stack_top = cb->stack_top++;

	if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
		cb->stack = realloc(cb->stack, sizeof *cb->stack * stack_top << 1);
	else if (!cb->stack)
		cb->stack = malloc(sizeof *cb->stack * UO_CB_STACK_MIN_ALLOC);

	cb->stack[stack_top] = ptr;
}

void *uo_cb_stack_pop(
	uo_cb *cb)
{
	size_t stack_top = --cb->stack_top;

	void *ptr = cb->stack[stack_top];

	if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
		cb->stack = realloc(cb->stack, sizeof *cb->stack * stack_top);

	return ptr;
}

void *uo_cb_stack_peek(
	uo_cb *cb)
{
	return cb->stack[cb->stack_top - 1];
}
