#include "uo_cb_queue.h"

#include <semaphore.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define UO_CB_QUEUE_CAPACITY 0x100
#define UO_CB_QUEUE_SEM_TIMEO 500.0

static bool is_init;

static inline void timespec_add_ms(
    struct timespec *ts, 
    double ms)
{
    bool cf = (ts->tv_nsec += fmod(ms * 1000000.0, 1000000000.0)) >= 1000000000;
    ts->tv_nsec -= cf * 1000000000;
    ts->tv_sec += (time_t)(ms / 1000.0) + cf;
}

static struct 
{
    sem_t dequeue_sem;
    sem_t enqueue_sem;
    atomic_size_t head;
    atomic_size_t tail;
    uo_cb *cbs[UO_CB_QUEUE_CAPACITY];
} cb_queue;

static void uo_cb_queue_quit(void)
{
    sem_destroy(&cb_queue.dequeue_sem);
    sem_destroy(&cb_queue.enqueue_sem);

    is_init = false;
}

bool uo_cb_queue_init()
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= sem_init(&cb_queue.dequeue_sem, 0, 0) == 0;
    is_init &= sem_init(&cb_queue.enqueue_sem, 0, UO_CB_QUEUE_CAPACITY) == 0;

    atomic_init(&cb_queue.head, 0);
    atomic_init(&cb_queue.tail, 0);

    atexit(uo_cb_queue_quit);

    return is_init;
}

void uo_cb_queue_enqueue(
    uo_cb *cb)
{
    sem_wait(&cb_queue.enqueue_sem);
    size_t head = atomic_fetch_add(&cb_queue.head, 1);
    cb_queue.cbs[head % UO_CB_QUEUE_CAPACITY] = cb;
    sem_post(&cb_queue.dequeue_sem);
}

uo_cb *uo_cb_queue_try_dequeue()
{
    struct timespec abs_timeout;
    clock_gettime(CLOCK_REALTIME, &abs_timeout);
    timespec_add_ms(&abs_timeout, UO_CB_QUEUE_SEM_TIMEO);

    if (sem_timedwait(&cb_queue.dequeue_sem, &abs_timeout) == 0)
    {
        size_t tail = atomic_fetch_add(&cb_queue.tail, 1);
        uo_cb *cb = cb_queue.cbs[tail % UO_CB_QUEUE_CAPACITY];

        sem_post(&cb_queue.enqueue_sem);

        return cb;
    }
    
    return NULL;
}
