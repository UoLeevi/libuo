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
    uo_stack_create_at(&cb->stack, 0);
    cb->func_list.items = malloc(sizeof *cb->func_list.items * UO_CB_FUNC_LIST_MIN_ALLOC);
    return cb;
}

uo_cb *uo_cb_clone(
    const uo_cb *cb)
{
    uo_cb *cb_clone = calloc(1, sizeof *cb);

    uo_stack_create_at(&cb_clone->stack, &cb->stack);
    uo_stack_push_arr(&cb_clone->stack, cb->stack.items, cb->stack.count);

    size_t count = cb_clone->func_list.count = cb->func_list.count;
    size_t func_list_size = next_power_of_two(count);
    func_list_size = func_list_size > UO_CB_FUNC_LIST_MIN_ALLOC ? func_list_size : UO_CB_FUNC_LIST_MIN_ALLOC;
    cb_clone->func_list.items = malloc(sizeof *cb_clone->func_list.items * func_list_size);
    memcpy(cb_clone->func_list.items, cb->func_list.items, sizeof *cb->func_list.items * count);

    return cb_clone;
}

void uo_cb_destroy(
    uo_cb *cb)
{
    uo_stack_destroy_at(&cb->stack);
    free(cb->func_list.items);
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

    size_t func_list_size = UO_CB_FUNC_LIST_MIN_ALLOC;
    while (func_list_size < count)
        func_list_size <<= 1;

    cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * func_list_size);

    memcpy(cb->func_list.items + cb->func_list.count, cb_before->func_list.items, sizeof *cb->func_list.items * cb_before->func_list.count);

    uo_stack_push_arr(&cb->stack, cb_before->stack.items, cb_before->stack.count);

    cb->func_list.count = count;
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

    size_t func_list_size = UO_CB_FUNC_LIST_MIN_ALLOC;
    while (func_list_size < count)
        func_list_size <<= 1;

    cb->func_list.items = realloc(cb->func_list.items, sizeof *cb->func_list.items * func_list_size);

    memmove(cb->func_list.items + cb_after->func_list.count, cb->func_list.items, sizeof *cb->func_list.items * cb->func_list.count);
    memcpy(cb->func_list.items, cb_after->func_list.items, sizeof *cb->func_list.items * cb_after->func_list.count);

    uo_stack_insert_arr(&cb->stack, 0, cb_after->stack.items, cb_after->stack.count);

    cb->func_list.count = count;
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
    uo_cb *cb)
{
    uo_cb_queue_enqueue(cb);
}
