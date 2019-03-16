#include "uo_finstack.h"
#include "uo_stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct uo_finstack_item
{
    void *ptr;
    void (*finalizer)(void *);
};

uo_finstack *uo_finstack_create()
{
    return uo_stack_create(0);
}

void uo_finstack_destroy(
    uo_finstack *finstack)
{
    uo_stack_destroy(finstack);
}

void uo_finstack_add(
    uo_finstack *finstack,
    void *ptr,
    void (*finalizer)(void *))
{
    struct uo_finstack_item *item = malloc(sizeof *item);
    item->ptr = ptr;
    item->finalizer = finalizer;
    uo_stack_push(finstack, item);
}

void uo_finstack_finalize(
    uo_finstack *finstack)
{
    size_t count = uo_stack_count(finstack);
    struct uo_finstack_item *item;

    for (size_t i = 0; i < count; ++i)
    {
        item = uo_stack_pop(finstack);
        
        if (item->finalizer)
            item->finalizer(item->ptr);

        free(item);
    }
}
