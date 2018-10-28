#include "uo_cb.h"
#include "uo_queue.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

static bool is_init;
static bool is_quitting;
static pthread_t *thrds;
static size_t thrds_len;
static uo_queue *cb_queue;

typedef struct uo_async_cb 
{
	uo_cb *cb;
	void *arg;
	sem_t *sem;
} uo_async_cb;

static void uo_cb_quit(void) 
{
	is_quitting = true;
	
	for (int i = 0; i < thrds_len; ++i)
		uo_cb_invoke_async(uo_cb_create(UO_CB_OPT_DESTROY), NULL, NULL);

	for (int i = 0; i < thrds_len; ++i)
		pthread_join(thrds[i], NULL);

	free(thrds);
	uo_queue_destroy(cb_queue);

	is_init = is_quitting = false;
}

static void *execute(
	void *arg) 
{
	while (!is_quitting) 
	{
		uo_async_cb *async_cb = uo_queue_dequeue(cb_queue, true);
		uo_cb_invoke(async_cb->cb, async_cb->arg);
		if (async_cb->sem)
			sem_post(async_cb->sem);
		free(async_cb);
	}

	return NULL;
}

bool uo_cb_init(
	size_t thrd_count) 
{
	if (is_init)
	{
		if (thrd_count > thrds_len) 
		{
			thrds = realloc(thrds, sizeof(pthread_t) * thrd_count);
			for (int i = thrds_len; is_init && i < thrd_count; ++i) 
				is_init &= pthread_create(thrds + i, NULL, execute, NULL) == 0;
			thrds_len = thrd_count;
		}

		return is_init;
	}

	is_init = true;
	thrds_len = thrd_count || 1;
	thrds = malloc(sizeof(pthread_t) * thrds_len);

	cb_queue = uo_queue_create(0x100);

	for (int i = 0; i < thrds_len; ++i) 
		is_init &= pthread_create(thrds + i, NULL, execute, NULL) == 0;

	atexit(uo_cb_quit);

	return is_init;
}

uo_cb *uo_cb_create(
	UO_CB_OPT opt)
{
	uo_cb *cb = calloc(1, sizeof *cb);
	cb->f = malloc(sizeof *cb->f * 2);
	cb->stack = malloc(sizeof *cb->stack * 2);
	cb->stack_capacity = 2;
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
	if (cb->count && !(cb->count & (cb->count - 1)))
		cb->f = realloc(cb->f, sizeof *cb->f * (cb->count * 2));

	cb->f[cb->count++] = f;
}

void uo_cb_prepend(
	uo_cb *cb,
	void *(*f)(void *arg, uo_cb *))
{
	if (cb->count && !(cb->count & (cb->count - 1)))
		cb->f = realloc(cb->f, sizeof *cb->f * (cb->count * 2));
	
	memmove(cb->f + 1, cb->f, sizeof *cb->f * cb->count++);
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

	uo_async_cb *async_cb = malloc(sizeof *async_cb);
	async_cb->cb = cb;
	async_cb->arg = arg;
	async_cb->sem = sem;

	uo_queue_enqueue(cb_queue, async_cb, true);
}

void uo_cb_push_data(
	uo_cb *cb, 
	void *data)
{
	if (cb->stack_top == cb->stack_capacity)
		cb->stack = realloc(cb->stack, sizeof *cb->stack * (cb->stack_capacity *= 2));
	
	cb->stack[cb->stack_top++] = data; 
}

void *uo_cb_pop_data(
	uo_cb *cb)
{
	return cb->stack[--cb->stack_top];
}

void *uo_cb_peek_data(
	uo_cb *cb)
{
	return cb->stack[cb->stack_top - 1];
}
