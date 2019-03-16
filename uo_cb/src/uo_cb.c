#include "uo_cb.h"
#include "uo_cb_queue.h"
#include "uo_cb_thrdpool.h"
#include "uo_stack.h"
#include "uo_linklist.h"
#include "uo_linkpool.h"

#include <pthread.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct uo_cb 
{
    uo_stack stack;
    uo_linklist funclist;
};

uo_def_linkpool(uo_cb);
uo_def_linkpool(uo_cb_func);

static bool is_init;

static void uo_cb_quit(void)
{
    uo_cb_func_linkpool_thrd_quit();
    uo_cb_linkpool_thrd_quit();
}

bool uo_cb_init() 
{
    if (is_init)
        return true;

    is_init = true;

    is_init &= uo_cb_linkpool_init();
    is_init &= uo_cb_func_linkpool_init();
    is_init &= uo_cb_linkpool_thrd_init();
    is_init &= uo_cb_func_linkpool_thrd_init();

    atexit(uo_cb_quit);

    is_init &= uo_cb_queue_init();
    is_init &= uo_cb_thrdpool_init();

    return is_init;
}

bool uo_cb_thrd_init()
{
    bool is_init = true;

    is_init &= uo_cb_linkpool_thrd_init();
    is_init &= uo_cb_func_linkpool_thrd_init();

    return is_init;
}

void uo_cb_thrd_quit()
{
    uo_cb_func_linkpool_thrd_quit();
    uo_cb_linkpool_thrd_quit();
}

uo_cb *uo_cb_create()
{
    uo_cb *cb = &uo_cb_linkpool_rent()->item;
    uo_stack_create_at(&cb->stack, 0);
    uo_linklist_selflink(&cb->funclist);

    return cb;
}

uo_cb *uo_cb_clone(
    const uo_cb *cb)
{
    uo_cb *cb_clone = uo_cb_create();
    uo_stack_push_arr(&cb_clone->stack, cb->stack.items, cb->stack.count);

    uo_linklist *link = cb->funclist.next;

    while (link != &cb->funclist)
    {
        uo_cb_append_func(cb_clone, ((uo_cb_func_linklist *)link)->item);
        link = link->next;
    }

    return cb_clone;
}

void uo_cb_destroy(
    uo_cb *cb)
{
    uo_stack_destroy_at(&cb->stack);

    while (!uo_linklist_is_empty(&cb->funclist))
    {
        uo_cb_func_linklist *cb_func_linklist = (uo_cb_func_linklist *)uo_linklist_next(&cb->funclist);
        uo_linklist_unlink(cb_func_linklist);
        uo_cb_func_linkpool_return(cb_func_linklist);
    }

    uo_cb_linkpool_return(uo_cb_get_linklist(cb));
}

void uo_cb_prepend_func(
    uo_cb *cb,
    uo_cb_func cb_func)
{
    uo_cb_func_linklist *cb_func_linklist = uo_cb_func_linkpool_rent();
    cb_func_linklist->item = cb_func;
    uo_linklist_link(cb->funclist.next, cb_func_linklist);
}

void uo_cb_prepend_cb(
    uo_cb *cb,
    uo_cb *cb_before)
{
    uo_stack_push_arr(&cb->stack, cb_before->stack.items, cb_before->stack.count);

    uo_linklist *link = cb_before->funclist.prev;

    while (link != &cb_before->funclist)
    {
        uo_cb_prepend_func(cb, ((uo_cb_func_linklist *)link)->item);
        link = link->prev;
    }
}

void uo_cb_append_func(
    uo_cb *cb,
    uo_cb_func cb_func)
{
    uo_cb_func_linklist *cb_func_linklist = uo_cb_func_linkpool_rent();
    cb_func_linklist->item = cb_func;
    uo_linklist_link(&cb->funclist, cb_func_linklist);
}

void uo_cb_append_cb(
    uo_cb *cb,
    uo_cb *cb_after)
{
    uo_stack_insert_arr(&cb->stack, 0, cb_after->stack.items, cb_after->stack.count);

    uo_linklist *link = cb_after->funclist.next;

    while (link != &cb_after->funclist)
    {
        uo_cb_append_func(cb, ((uo_cb_func_linklist *)link)->item);
        link = link->next;
    }
}

void uo_cb_invoke(
    uo_cb *cb)
{
    if (!uo_linklist_is_empty(&cb->funclist))
    {
        uo_cb_func_linklist *cb_func_linklist = (uo_cb_func_linklist *)uo_linklist_next(&cb->funclist);
        uo_linklist_unlink(cb_func_linklist);
        uo_cb_func cb_func = cb_func_linklist->item;
        uo_cb_func_linkpool_return(cb_func_linklist);
        cb_func(cb);
    }
    else
        uo_cb_destroy(cb);
}

void uo_cb_invoke_async(
    uo_cb *cb)
{
    uo_cb_queue_enqueue(cb);
}

inline void uo_cb_stack_push(
    uo_cb *cb,
    void *item)
{
    uo_stack_push(&cb->stack, item);
}

inline void *uo_cb_stack_pop(
    uo_cb *cb)
{
    return uo_stack_pop(&cb->stack);
}

inline void *uo_cb_stack_peek(
    uo_cb *cb)
{
    return uo_stack_peek(&cb->stack);
}

inline void *uo_cb_stack_index(
    uo_cb *cb,
    int index)
{
    return uo_stack_index(&cb->stack, index);
}
