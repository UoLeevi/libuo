#include "uo_stack.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define UO_STACK_MIN_CAPACITY 0x4

uo_stack *uo_stack_create(
    size_t initial_capacity)
{
    size_t capacity = UO_STACK_MIN_CAPACITY;

    while (capacity <= initial_capacity)
        capacity <<= 1;

    uo_stack *stack = calloc(1, sizeof *stack);
    stack->items = malloc(sizeof *stack->items * (stack->capacity = capacity));

    return stack;
}

void uo_stack_destroy(
    uo_stack *stack)
{
    free(stack->items);
    free(stack);
}

void uo_stack_push(
    uo_stack *stack,
    const void *item)
{
    size_t count = stack->count++;

    if (count == stack->capacity)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity <<= 1));

    stack->items[count] = (void *)item;
}

void *uo_stack_pop(
    uo_stack *stack)
{
    assert(stack->count);

    size_t count = --stack->count;
    void *item = stack->items[count];

    if ((count == stack->capacity >> 3) >= UO_STACK_MIN_CAPACITY)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity >>= 1));

    return item;
}
