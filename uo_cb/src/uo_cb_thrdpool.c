#include "uo_cb_thrdpool.h"
#include "uo_cb_queue.h"
#include "uo_linkpool.h"

#include <pthread.h>
#include <semaphore.h>
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

#define UO_CB_THRDPOOL_TIMEO_INI 500.0
#define UO_CB_THRDPOOL_TIMEO_MIN 10.0
#define UO_CB_THRDPOOL_TIMEO_MAX 5000.0
#define UO_CB_THRDPOOL_TIMEO_INC_FACTOR 1.5
#define UO_CB_THRDPOOL_TIMEO_DEC_FACTOR 0.8

#define UO_CB_THRDPOOL_COST_PER_THRD 0.05
#define UO_CB_THRDPOOL_PAUSE_COUNTER 100

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
    atomic_bool is_paused;
    sem_t update_sem;
    pthread_t manager_thrd;
    atomic_size_t thrd_count;
    atomic_size_t executing_count;
    size_t thrd_count_target;
    struct
    {
        pthread_t thrd;
        enum UO_CB_THRD_STATE state;

    } cb_thrds[UO_CB_THRD_COUNT_MAX];
    struct
    {
        atomic_size_t completions;
        double weighted_throughput;
        bool thrd_target_increase;
    } sample;
} thrdpool;

static inline void timespec_add_ms(
    struct timespec *ts, 
    double ms)
{
    bool cf = (ts->tv_nsec += fmod(ms * 1000000.0, 1000000000.0)) >= 1000000000;
    ts->tv_nsec -= cf * 1000000000;
    ts->tv_sec += (time_t)(ms / 1000.0) + cf;
}

static inline void uo_cb_thrdpool_notify_before_invoke(void)
{
    atomic_fetch_add(&thrdpool.executing_count, 1);
}

static inline void uo_cb_thrdpool_notify_after_invoke(void)
{
    atomic_fetch_sub(&thrdpool.executing_count, 1);
    atomic_fetch_add(&thrdpool.sample.completions, 1);

    bool is_paused = atomic_exchange(&thrdpool.is_paused, false);
    
    if (is_paused)
        sem_post(&thrdpool.update_sem);
}

static void *uo_cb_execute(
    void *thrd_i)
{
    uo_cb_thrd_init();

    while (!is_quitting)
    {
        uo_cb *cb = uo_cb_queue_try_dequeue();

        if (cb)
        {
            uo_cb_thrdpool_notify_before_invoke();
            uo_cb_invoke(cb);
            uo_cb_thrdpool_notify_after_invoke();
        }

        size_t count = atomic_load(&thrdpool.thrd_count);
        size_t count_target = thrdpool.thrd_count_target;

        if (count > count_target && atomic_compare_exchange_strong(&thrdpool.thrd_count, &count, count - 1))
            break;
    }

    uo_cb_thrd_quit();

    thrdpool.cb_thrds[(size_t)thrd_i].state = UO_CB_THRD_EXITED;
}

static void *uo_cb_manage_thrds(
    void *arg)
{
    thrdpool.thrd_count_target = UO_CB_THRD_COUNT_INI;

    size_t thrd_i = 0;
    size_t count = thrdpool.thrd_count;

    double duration_msec = UO_CB_THRDPOOL_TIMEO_INI;
    size_t pause_counter = UO_CB_THRDPOOL_PAUSE_COUNTER;

    while (!is_quitting)
    {
        while (!is_quitting && count < thrdpool.thrd_count_target && count < UO_CB_THRD_COUNT_MAX)
        {
            count = atomic_fetch_add(&thrdpool.thrd_count, 1) + 1;

            enum UO_CB_THRD_STATE thrd_state;

            while (thrd_state = thrdpool.cb_thrds[thrd_i].state)
            {
                if (thrd_state == UO_CB_THRD_EXITED)
                {
                    pthread_join(thrdpool.cb_thrds[thrd_i].thrd, NULL);
                    thrdpool.cb_thrds[thrd_i].state = UO_CB_THRD_UNINITIALIZED;
                }

                ++thrd_i;
                thrd_i %= UO_CB_THRD_COUNT_MAX;
            }

            thrdpool.cb_thrds[thrd_i].state = UO_CB_THRD_RUNNING;
            
            pthread_create(&thrdpool.cb_thrds[thrd_i].thrd, 0, uo_cb_execute, (void *)(uintptr_t)thrd_i);
        }

        struct timespec abs_timeout;
        clock_gettime(CLOCK_REALTIME, &abs_timeout);
        timespec_add_ms(&abs_timeout, duration_msec);

        if (sem_timedwait(&thrdpool.update_sem, &abs_timeout) != 0)
        {
            if (atomic_load(&thrdpool.executing_count))
            {
                size_t completions = atomic_exchange(&thrdpool.sample.completions, 0);

                size_t prev_weighted_throughput = thrdpool.sample.weighted_throughput;
                thrdpool.sample.weighted_throughput = completions / duration_msec * pow(1.0 - UO_CB_THRDPOOL_COST_PER_THRD, thrdpool.thrd_count_target);

                bool was_correct = thrdpool.sample.weighted_throughput > prev_weighted_throughput;
                thrdpool.sample.thrd_target_increase = !(was_correct ^ thrdpool.sample.thrd_target_increase);
                
                if (thrdpool.sample.thrd_target_increase)
                {
                    if (thrdpool.thrd_count_target < UO_CB_THRD_COUNT_MAX)
                    {
                        ++thrdpool.thrd_count_target;
                        duration_msec *= was_correct ? UO_CB_THRDPOOL_TIMEO_DEC_FACTOR : UO_CB_THRDPOOL_TIMEO_INC_FACTOR;
                    }
                    else
                        duration_msec *= UO_CB_THRDPOOL_TIMEO_INC_FACTOR;
                }
                else
                {
                    if (thrdpool.thrd_count_target > UO_CB_THRD_COUNT_MIN)
                    {
                        --thrdpool.thrd_count_target;
                        duration_msec *= was_correct ? UO_CB_THRDPOOL_TIMEO_DEC_FACTOR : UO_CB_THRDPOOL_TIMEO_INC_FACTOR;
                    }
                    else
                        duration_msec *= UO_CB_THRDPOOL_TIMEO_INC_FACTOR;
                }
            }
            else
            {
                if (--pause_counter == 0)
                {
                    if (thrdpool.thrd_count_target > UO_CB_THRD_COUNT_MIN)
                    {
                        --thrdpool.thrd_count_target;
                        duration_msec *= UO_CB_THRDPOOL_TIMEO_INC_FACTOR;
                    }
                    else
                        sem_wait(&thrdpool.update_sem);

                    pause_counter = UO_CB_THRDPOOL_PAUSE_COUNTER;
                }
                
                atomic_store(&thrdpool.sample.completions, 0);
            }

            duration_msec = duration_msec > UO_CB_THRDPOOL_TIMEO_MAX
                ? UO_CB_THRDPOOL_TIMEO_MAX
                : duration_msec < UO_CB_THRDPOOL_TIMEO_MIN
                    ? UO_CB_THRDPOOL_TIMEO_MIN
                    : duration_msec;
        }
    }

    return NULL;
}

static void uo_cb_thrdpool_quit(void)
{
    is_quitting = true;

    uo_cb_thrd_init();
    sem_post(&thrdpool.update_sem);

    pthread_join(thrdpool.manager_thrd, NULL);

    for (size_t i = 0; i < thrdpool.thrd_count; ++i)
        uo_cb_invoke_async(uo_cb_create());

    for (size_t i = 0; i < UO_CB_THRD_COUNT_MAX; ++i)
        if (thrdpool.cb_thrds[i].state)
            pthread_join(thrdpool.cb_thrds[i].thrd, NULL);

    sem_destroy(&thrdpool.update_sem);
    uo_cb_thrd_quit();

    is_init = false;
}

bool uo_cb_thrdpool_init()
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= sem_init(&thrdpool.update_sem, 0, 0) == 0;

    atomic_init(&thrdpool.is_paused, false);
    atomic_init(&thrdpool.thrd_count, 0);
    atomic_init(&thrdpool.executing_count, 0);
    atomic_init(&thrdpool.sample.completions, 0);

    is_init = pthread_create(&thrdpool.manager_thrd, NULL, uo_cb_manage_thrds, NULL) == 0;

    atexit(uo_cb_thrdpool_quit);

    return is_init;
}

