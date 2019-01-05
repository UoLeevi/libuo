#include "uo_cb_stack.h"

#include <stdlib.h>

void uo_cb_stack_push_stack(
    uo_cb_stack *stack,
    void *ptr)
{
    size_t stack_top = stack->top++;

    if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
        stack->items = realloc(stack->items, sizeof *stack->items * stack_top << 1);
    else if (!stack->items)
        stack->items = malloc(sizeof *stack->items * UO_CB_STACK_MIN_ALLOC);

    stack->items[stack_top] = ptr;
}

void *uo_cb_stack_pop_stack(
    uo_cb_stack *stack)
{
    size_t stack_top = --stack->top;

    void *ptr = stack->items[stack_top];

    if (stack_top >= UO_CB_STACK_MIN_ALLOC && !(stack_top & (stack_top - 1)))
        stack->items = realloc(stack->items, sizeof *stack->items * stack_top);

    return ptr;
}

void *uo_cb_stack_peek_stack(
    uo_cb_stack *stack)
{
    return stack->items[stack->top - 1];
}

void *uo_cb_stack_index_stack(
    uo_cb_stack *stack,
    int index)
{
    return stack->items[index >= 0 ? index : (stack->top + index)];
}
