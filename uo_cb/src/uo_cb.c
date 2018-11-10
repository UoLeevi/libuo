#include "uo_cb.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdatomic.h>

#define UO_CB_THRD_COUNT_INI 4
#define UO_CB_THRD_COUNT_MIN 2
#define UO_CB_THRD_COUNT_MAX 16

#define UO_CB_THRD_POOL_TIMEO_INI 500.0
#define UO_CB_THRD_POOL_TIMEO_MIN 10.0
#define UO_CB_THRD_POOL_TIMEO_MAX 5000.0
#define UO_CB_THRD_POOL_TIMEO_INC_FACTOR 1.5
#define UO_CB_THRD_POOL_TIMEO_DEC_FACTOR 0.8

#define UO_CB_QUEUE_CAPACITY 0x100
#define UO_CB_QUEUE_SEM_TIMEO 500.0

#define UO_CB_F_MIN_ALLOC 2
#define UO_CB_STACK_MIN_ALLOC 2

static bool is_init;
static bool is_quitting;

enum UO_CB_THRD_STATE
{
	UO_CB_THRD_UNINITIALIZED = 0,
	UO_CB_THRD_RUNNING = 1 << 0,
	UO_CB_THRD_EXITED = 1 << 1,
} UO_CB_THRD_STATE;

static struct
{
	sem_t quit_sem;
	pthread_t manager_thrd;
	atomic_size_t thrd_count;
	size_t thrd_count_target;
	struct
	{
		pthread_t thrd;
		enum UO_CB_THRD_STATE state;

	} cb_thrds[UO_CB_THRD_COUNT_MAX];
	struct
	{
		atomic_size_t completions;
		double throughput;
		bool thrd_target_increase;
	} sample;
} thrd_pool;

static struct 
{
	sem_t dequeue_sem;
	sem_t enqueue_sem;
	atomic_size_t head;
	atomic_size_t tail;
	uo_cb *cbs[UO_CB_QUEUE_CAPACITY];
} cb_queue;

static inline void timespec_add_ms(
	struct timespec *ts, 
	double ms)
{
	ts->tv_nsec += fmod(ms * 1000000.0, 1000000000.0);
	ts->tv_sec += (uint64_t)ms / 1000 + (ts->tv_nsec >= 1000000000);
	ts->tv_nsec %= 1000000000;
}

static void *uo_cb_execute(
	void *thrd_i)
{
	while (!is_quitting)
	{
		struct timespec abs_timeout;
		clock_gettime(CLOCK_REALTIME, &abs_timeout);
		timespec_add_ms(&abs_timeout, UO_CB_QUEUE_SEM_TIMEO);

		while (sem_timedwait(&cb_queue.dequeue_sem, &abs_timeout) == 0)
		{
			size_t tail = atomic_fetch_add(&cb_queue.tail, 1);
			uo_cb *cb = cb_queue.cbs[tail % UO_CB_QUEUE_CAPACITY];

			sem_post(&cb_queue.enqueue_sem);

			sem_t *sem = uo_cb_stack_pop(cb);
			void *arg = uo_cb_stack_pop(cb);
			
			uo_cb_invoke(cb, arg);

			if (sem)
				sem_post(sem);

			atomic_fetch_add(&thrd_pool.sample.completions, 1);
		}
	
		size_t count = atomic_load(&thrd_pool.thrd_count);
		size_t count_target = thrd_pool.thrd_count_target;
		
		if (count > count_target && atomic_compare_exchange_strong(&thrd_pool.thrd_count, &count, count - 1))
			break;
	}

	thrd_pool.cb_thrds[(size_t)thrd_i].state = UO_CB_THRD_EXITED;
}


static void *uo_cb_manage_thrds(
	void *arg)
{
	thrd_pool.thrd_count_target = UO_CB_THRD_COUNT_INI;

	size_t thrd_i = 0;
	size_t count = thrd_pool.thrd_count;

	double duration_msec = UO_CB_THRD_POOL_TIMEO_INI;

	while (!is_quitting)
	{
		while (!is_quitting && count < thrd_pool.thrd_count_target && count < UO_CB_THRD_COUNT_MAX)
		{
			count = atomic_fetch_add(&thrd_pool.thrd_count, 1) + 1;

			enum UO_CB_THRD_STATE thrd_state;

			while (thrd_state = thrd_pool.cb_thrds[thrd_i].state)
			{
				if (thrd_state == UO_CB_THRD_EXITED)
				{
					pthread_join(thrd_pool.cb_thrds[thrd_i].thrd, NULL);
					thrd_pool.cb_thrds[thrd_i].state = UO_CB_THRD_UNINITIALIZED;
				}

				++thrd_i;
				thrd_i %= UO_CB_THRD_COUNT_MAX;
			}

			thrd_pool.cb_thrds[thrd_i].state = UO_CB_THRD_RUNNING;
			
			pthread_create(&thrd_pool.cb_thrds[thrd_i].thrd, 0, uo_cb_execute, (void *)(uintptr_t)thrd_i);
		}

		struct timespec abs_timeout;
		clock_gettime(CLOCK_REALTIME, &abs_timeout);
		timespec_add_ms(&abs_timeout, duration_msec);

		if (sem_timedwait(&thrd_pool.quit_sem, &abs_timeout) != 0)
		{
			size_t completions = atomic_exchange(&thrd_pool.sample.completions, 0);
			int cb_queue_count;
			sem_getvalue(&cb_queue.dequeue_sem, &cb_queue_count);

			if (completions || cb_queue_count > 0)
			{
				size_t prev_throughput = thrd_pool.sample.throughput;
				thrd_pool.sample.throughput = completions / duration_msec;

				bool was_correct = thrd_pool.sample.throughput > prev_throughput;
				thrd_pool.sample.thrd_target_increase = !(was_correct ^ thrd_pool.sample.thrd_target_increase);
				
				if (thrd_pool.sample.thrd_target_increase)
				{
					if (thrd_pool.thrd_count_target < UO_CB_THRD_COUNT_MAX)
					{
						++thrd_pool.thrd_count_target;
						duration_msec *= was_correct ? UO_CB_THRD_POOL_TIMEO_DEC_FACTOR : UO_CB_THRD_POOL_TIMEO_INC_FACTOR;
					}
					else
						duration_msec *= UO_CB_THRD_POOL_TIMEO_INC_FACTOR;
				}
				else
				{
					if (thrd_pool.thrd_count_target > UO_CB_THRD_COUNT_MIN)
					{
						--thrd_pool.thrd_count_target;
						duration_msec *= was_correct ? UO_CB_THRD_POOL_TIMEO_DEC_FACTOR : UO_CB_THRD_POOL_TIMEO_INC_FACTOR;
					}
					else
						duration_msec *= UO_CB_THRD_POOL_TIMEO_INC_FACTOR;
				}

				duration_msec = duration_msec > UO_CB_THRD_POOL_TIMEO_MAX
					? UO_CB_THRD_POOL_TIMEO_MAX
					: duration_msec < UO_CB_THRD_POOL_TIMEO_MIN
						? UO_CB_THRD_POOL_TIMEO_MIN
						: duration_msec;
			}
		}
	}

	return NULL;
}

static void uo_cb_quit(void) 
{
	is_quitting = true;
	sem_post(&thrd_pool.quit_sem);

	pthread_join(thrd_pool.manager_thrd, NULL);
	
	for (size_t i = 0; i < thrd_pool.thrd_count; ++i)
		uo_cb_invoke_async(uo_cb_create(UO_CB_OPT_DESTROY), NULL, NULL);

	for (size_t i = 0; i < UO_CB_THRD_COUNT_MAX; ++i)
		if (thrd_pool.cb_thrds[i].state)
			pthread_join(thrd_pool.cb_thrds[i].thrd, NULL);

	is_init = is_quitting = false;
}

bool uo_cb_init() 
{
	if (is_init)
		return true;

	sem_init(&cb_queue.dequeue_sem, 0, 0);
	sem_init(&cb_queue.enqueue_sem, 0, UO_CB_QUEUE_CAPACITY);

	atomic_init(&cb_queue.head, 0);
	atomic_init(&cb_queue.tail, 0);

	sem_init(&thrd_pool.quit_sem, 0, 0);

	atomic_init(&thrd_pool.thrd_count, 0);
	atomic_init(&thrd_pool.sample.completions, 0);

	is_init = pthread_create(&thrd_pool.manager_thrd, NULL, uo_cb_manage_thrds, NULL) == 0;

	atexit(uo_cb_quit);

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

	sem_wait(&cb_queue.enqueue_sem);
	
	size_t head = atomic_fetch_add(&cb_queue.head, 1);
	cb_queue.cbs[head % UO_CB_QUEUE_CAPACITY] = cb;

	sem_post(&cb_queue.dequeue_sem);
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
