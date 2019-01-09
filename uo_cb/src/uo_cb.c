#include "uo_cb.h"
#include "uo_cb_queue.h"
#include "uo_cb_thrd_pool.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define UO_CB_FUNC_LIST_MIN_ALLOC 2
#define UO_CB_STACK_MIN_ALLOC 2

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
    cb->func_list.items = malloc(sizeof *cb->func_list.items * UO_CB_FUNC_LIST_MIN_ALLOC);
    return cb;
}

uo_cb *uo_cb_clone(
    const uo_cb *cb)
{
    uo_cb *cb_clone = calloc(1, sizeof *cb);

    size_t count = cb_clone->func_list.count = cb->func_list.count;
    size_t func_list_size = next_power_of_two(count);
    func_list_size = func_list_size > UO_CB_FUNC_LIST_MIN_ALLOC ? func_list_size : UO_CB_FUNC_LIST_MIN_ALLOC;
    cb_clone->func_list.items = malloc(sizeof *cb_clone->func_list.items * func_list_size);
    memcpy(cb_clone->func_list.items, cb->func_list.items, sizeof *cb->func_list.items * count);

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
    free(cb->func_list.items);
    free(cb->stack.items);
    free(cb);
}

void uo_cb_prepend_func(
    uo_cb *cb,
    uo_cb_func *cb_func)
{
    size_t count = cb->func_list.count++;

    if (count >= UO_CB_FUNC_LIST_MIN_ALLOC && !(count & (count - 1)))
        cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * count << 1);

    cb->func_list.items[count] = cb_func;
}

void uo_cb_prepend_cb(
    uo_cb *cb,
    uo_cb *cb_before)
{
    size_t count = cb->func_list.count + cb_before->func_list.count;
    size_t stack_top = cb->stack.top + cb_before->stack.top;

    size_t func_list_size = UO_CB_FUNC_LIST_MIN_ALLOC;
    while (func_list_size < count)
        func_list_size <<= 1;

    cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * func_list_size);

    memcpy(cb->func_list.items + cb->func_list.count, cb_before->func_list.items, cb_before->func_list.count);

    size_t stack_size = UO_CB_STACK_MIN_ALLOC;
    while (stack_size < stack_top)
        stack_size <<= 1;

    cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_size);

    memmove(cb->stack.items + cb_before->stack.top, cb->stack.items, sizeof *cb->stack.items * cb->stack.top);
    memcpy(cb->stack.items, cb_before->stack.items, cb_before->stack.top);
}

void uo_cb_append_func(
    uo_cb *cb,
    uo_cb_func *cb_func)
{
    size_t count = cb->func_list.count++;

    if (count >= UO_CB_FUNC_LIST_MIN_ALLOC && !(count & (count - 1)))
        cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * count << 1);
    
    memmove(cb->func_list.items + 1, cb->func_list.items, sizeof *cb->func_list.items * count);
    *cb->func_list.items = cb_func;
}

void uo_cb_append_cb(
    uo_cb *cb,
    uo_cb *cb_after)
{
    size_t count = cb->func_list.count + cb_after->func_list.count;
    size_t stack_top = cb->stack.top + cb_after->stack.top;

    size_t func_list_size = UO_CB_FUNC_LIST_MIN_ALLOC;
    while (func_list_size < count)
        func_list_size <<= 1;

    cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * func_list_size);

    memmove(cb->func_list.items + cb_after->func_list.count, cb->func_list.items, sizeof *cb->func_list.items * cb->func_list.count);
    memcpy(cb->func_list.items, cb_after->func_list.items, cb_after->func_list.count);

    size_t stack_size = UO_CB_STACK_MIN_ALLOC;
    while (stack_size < stack_top)
        stack_size <<= 1;

    cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_size);

    memcpy(cb->stack.items + cb->stack.top, cb_after->stack.items, cb_after->stack.top);
}

void uo_cb_invoke(
    uo_cb *cb)
{
    size_t count = cb->func_list.count--;

    if (count)
        cb->func_list.items[count - 1](cb);
    else
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

void uo_cb_stack_push(
    uo_cb *cb,
    void *ptr)
{
    size_t stack_top = cb->stack.top++;

    if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
        cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_top << 1);
    else if (!cb->stack.items)
        cb->stack.items = malloc(sizeof *cb->stack.items * UO_CB_STACK_MIN_ALLOC);

    cb->stack.items[stack_top] = ptr;
}

void *uo_cb_stack_pop(
    uo_cb *cb)
{
    size_t stack_top = --cb->stack.top;

    void *ptr = cb->stack.items[stack_top];

    if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
        cb->stack.items = realloc(cb->stack.items, sizeof *cb->stack.items * stack_top);

    return ptr;
}

void *uo_cb_stack_peek(
    uo_cb *cb)
{
    return cb->stack.items[cb->stack.top - 1];
}

void *uo_cb_stack_index(
    uo_cb *cb,
    int index)
{
    return cb->stack.items[index >= 0 ? index : (cb->stack.top + index)];
}
