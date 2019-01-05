#include "uo_cb.h"
#include "uo_cb_queue.h"
#include "uo_cb_thrd_pool.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static bool is_init;

static inline uint64_t next_power_of_two(
    uint64_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    return n;
}

bool uo_cb_init() 
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= uo_cb_queue_init();
    is_init &= uo_cb_thrd_pool_init();

    return is_init;
}

uo_cb *uo_cb_create()
{
    uo_cb *cb = calloc(1, sizeof *cb);
    cb->f = malloc(sizeof *cb->f * UO_CB_F_MIN_ALLOC);
    return cb;
}

uo_cb *uo_cb_clone(
    const uo_cb *cb)
{
    uo_cb *cb_clone = calloc(1, sizeof *cb);

    size_t count = cb_clone->count = cb->count;
    size_t f_size = next_power_of_two(count);
    f_size = f_size > UO_CB_F_MIN_ALLOC ? f_size : UO_CB_F_MIN_ALLOC;
    cb_clone->f = malloc(sizeof *cb_clone->f * f_size);
    memcpy(cb_clone->f, cb->f, sizeof *cb->f * count);

    if (cb->stack.items)
    {
        size_t stack_top = cb_clone->stack.top = cb->stack.top;
        size_t stack_size = next_power_of_two(stack_top);
        stack_size = stack_size > UO_CB_STACK_MIN_ALLOC ? stack_size : UO_CB_STACK_MIN_ALLOC;
        cb_clone->stack.items = malloc(sizeof *cb_clone->stack.items * stack_size);
        memcpy(cb_clone->stack.items, cb->stack.items, sizeof *cb->stack.items * stack_top);
    }

    return cb_clone;
}

void uo_cb_destroy(
    uo_cb *cb)
{
    free(cb->f);
    free(cb->stack.items);
    free(cb);
}

void uo_cb_append_f(
    uo_cb *cb,
    void (*f)(uo_cb_stack *))
{
    size_t count = cb->count++;

    if (count >= UO_CB_F_MIN_ALLOC && !(count & (count - 1)))
        cb->f = realloc(cb->f, sizeof *cb->f * count << 1);

    cb->f[count] = f;
}

void uo_cb_append_cb(
    uo_cb *cb,
    uo_cb *cb_after)
{
    size_t count = cb->count + cb_after->count;
    size_t stack_top = cb->stack.top + cb_after->stack.top;

    size_t f_size = UO_CB_F_MIN_ALLOC;
    while (f_size < count)
        f_size <<= 1;

    cb->f = realloc(cb->f, sizeof *cb->f * f_size);

    memcpy(cb->f + cb->count, cb_after->f, cb_after->count);

    size_t stack_size = UO_CB_STACK_MIN_ALLOC;
    while (stack_size < stack_top)
        stack_size <<= 1;

    cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_size);

    memmove(cb->stack.items + cb_after->stack.top, cb->stack.items, sizeof *cb->stack.items * cb->stack.top);
    memcpy(cb->stack.items, cb_after->stack.items, cb_after->stack.top);
}

void uo_cb_prepend_f(
    uo_cb *cb,
    void (*f)(uo_cb_stack *))
{
    size_t count = cb->count++;

    if (count >= UO_CB_F_MIN_ALLOC && !(count & (count - 1)))
        cb->f = realloc(cb->f, sizeof *cb->f * count << 1);
    
    memmove(cb->f + 1, cb->f, sizeof *cb->f * count);
    *cb->f = f;
}

void uo_cb_prepend_cb(
    uo_cb *cb,
    uo_cb *cb_before)
{
    size_t count = cb->count + cb_before->count;
    size_t stack_top = cb->stack.top + cb_before->stack.top;

    size_t f_size = UO_CB_F_MIN_ALLOC;
    while (f_size < count)
        f_size <<= 1;

    cb->f = realloc(cb->f, sizeof *cb->f * f_size);

    memmove(cb->f + cb_before->count, cb->f, sizeof *cb->f * cb->count);
    memcpy(cb->f, cb_before->f, cb_before->count);

    size_t stack_size = UO_CB_STACK_MIN_ALLOC;
    while (stack_size < stack_top)
        stack_size <<= 1;

    cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_size);

    memcpy(cb->stack.items + cb->stack.top, cb_before->stack.items, cb_before->stack.top);
}

void uo_cb_invoke(
    uo_cb *cb)
{
    for (size_t i = 0; i < cb->count; ++i)
        cb->f[i](&cb->stack);

    uo_cb_destroy(cb);
}

void uo_cb_invoke_async(
    uo_cb *cb,
    sem_t *sem)
{
    if (sem) 
        sem_init(sem, 0, 0);
    
    uo_cb_stack_push(cb, sem);

    uo_cb_queue_enqueue(cb);
}
