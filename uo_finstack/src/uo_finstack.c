#include "uo_finstack.h"
#include "uo_stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct uo_finstack_item
{
    void *ptr;
    void (*ptr_finstack)(void *);
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

void uo__finstack_add(
    uo_finstack *finstack,
    void *ptr,
    void (*ptr_finstack)(void *))
{
    struct uo_finstack_item *item = malloc(sizeof *item);
    item->ptr = ptr;
    item->ptr_finstack = ptr_finstack;
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
        
        if (item->ptr_finstack)
            item->ptr_finstack(item->ptr);

        free(item);
    }
}
