#include "uo_cb.h"
#include <uo_queue.h>
#include <uo_hashtbl.h>

#include <stdbool.h>
#include <stdlib.h>

#include <pthread.h>

#define UO_F void *(*)(void *, void *)
#define UO_FSIZE sizeof(UO_F)

static bool is_init;
static bool is_quitting;
static pthread_t *thrds;
static size_t thrds_len;
static uo_queue *cb_queue;
static uo_hashtbl *cb_map;

typedef struct uo_async_cb {
	uo_cb *cb;
	void *arg;
	void *state;
	sem_t *sem;
} uo_async_cb;

static uint64_t addrofstate(
	const void *state)
{
	return (uintptr_t)state;
}

static bool stateequals(
	const void *state0,
	const void *state1)
{
	return addrofstate(state0) == addrofstate(state1);
} 

static void uo_cb_quit(void) 
{
	is_quitting = true;
	
	for (int i = 0; i < thrds_len; ++i)
		uo_cb_invoke_async(uo_cb_create(UO_CB_OPT_DESTROY), NULL, NULL, NULL);

	for (int i = 0; i < thrds_len; ++i)
		pthread_join(thrds[i], NULL);

	free(thrds);
	uo_queue_destroy(cb_queue);
	uo_hashtbl_destroy(cb_map);

	is_init = is_quitting = false;
}

void *uo_cb_invoke_func(
	void *arg,
	void *state)
{
	uo_cb *cb = uo_hashtbl_find(cb_map, state);

	if (cb->opt & UO_CB_OPT_DESTROY)
		uo_hashtbl_remove(cb_map, state);
		
	return uo_cb_invoke(cb, arg, state);
}

static void *execute(
	void *arg) 
{
	while (!is_quitting) 
	{
		uo_async_cb *async_cb = uo_queue_dequeue(cb_queue, true);
		uo_cb_invoke(async_cb->cb, async_cb->arg, async_cb->state);
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
	cb_map = uo_hashtbl_create(0x100, addrofstate, stateequals);

	for (int i = 0; i < thrds_len; ++i) 
		is_init &= pthread_create(thrds + i, NULL, execute, NULL) == 0;

	atexit(uo_cb_quit);

	return is_init;
}

uo_cb *uo_cb_create(
	UO_CB_OPT opt)
{
	uo_cb *cb = malloc(sizeof(uo_cb));
	cb->count = 0;
	cb->f = malloc(UO_FSIZE * 2);
	cb->opt = opt;
	return cb;
}

void uo_cb_destroy(
	uo_cb *cb)
{
	free(cb->f);
	free(cb);
}

void uo_cb_append(
	uo_cb *cb,
	void *(*f)(void *arg, void *state))
{
	if (cb->count && !(cb->count & (cb->count - 1)))
		cb->f = realloc(cb->f, (cb->count << 1) * UO_FSIZE);

	cb->f[cb->count] = f;

	++cb->count;
}

void *uo_cb_invoke(
	uo_cb *cb,
	void *arg,
	void *state)
{
	void *result = arg;

	for (size_t i = 0; i < cb->count; ++i)
		result = cb->f[i](result, state);

	if (cb->opt & UO_CB_OPT_DESTROY)
		uo_cb_destroy(cb);

	return result;
}

void uo_cb_invoke_async(
	uo_cb *cb,
	void *arg,
	void *state,
	sem_t *sem)
{
	if (sem)
		sem_init(sem, 0, 0);

	uo_async_cb *async_cb = malloc(sizeof(uo_async_cb));
	async_cb->cb = cb;
	async_cb->arg = arg;
	async_cb->state = state;
	async_cb->sem = sem;

	uo_queue_enqueue(cb_queue, async_cb, true);
}

void *(*uo_cb_as_func(
	uo_cb *cb, 
	void *state))(void *arg, void *state)
{
	uo_hashtbl_insert(cb_map, state, cb);
	return uo_cb_invoke_func;
}
