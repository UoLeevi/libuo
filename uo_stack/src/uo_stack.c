#include "uo_stack.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define UO_STACK_MIN_CAPACITY 0x4

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

void uo_stack_create_at(
    uo_stack *stack,
    size_t initial_capacity)
{
    size_t capacity = initial_capacity >= UO_STACK_MIN_CAPACITY
        ? next_power_of_two(initial_capacity)
        : UO_STACK_MIN_CAPACITY;

    stack->count = 0;
    stack->items = malloc(sizeof *stack->items * (stack->capacity = capacity));
}

uo_stack *uo_stack_create(
    size_t initial_capacity)
{
    uo_stack *stack = malloc(sizeof *stack);
    uo_stack_create_at(stack, initial_capacity);
    return stack;
}

void uo_stack_destroy_at(
    uo_stack *stack)
{
    free(stack->items);
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

void uo_stack_push_arr(
    uo_stack *stack,
    void *const *items,
    size_t count)
{
    size_t stack_count = count + stack->count;

    if (stack_count > stack->capacity)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity = next_power_of_two(stack_count)));

    memcpy(stack->items + stack->count, items, sizeof *items * count);

    stack->count = stack_count;
}

void *uo_stack_pop(
    uo_stack *stack)
{
    assert(stack->count);

    size_t count = --stack->count;
    void *item = stack->items[count];

    if (count == stack->capacity >> 3 && count >= UO_STACK_MIN_CAPACITY)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity >>= 1));

    return item;
}

void *uo_stack_peek(
    uo_stack *stack)
{
    return stack->items[stack->count - 1];
}

void *uo_stack_index(
    uo_stack *stack,
    int index)
{
    return stack->items[index >= 0 ? index : (stack->count + index)];
}

void uo_stack_insert(
    uo_stack *stack,
    int index,
    const void *item)
{
    index = index >= 0 ? index : (stack->count + index);

    size_t count = stack->count++;

    if (count == stack->capacity)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity <<= 1));

    memmove(stack->items + index + 1, stack->items + index, sizeof *stack->items * (count - index));

    stack->items[index] = (void *)item;
}

void uo_stack_insert_arr(
    uo_stack *stack,
    int index,
    void *const *items,
    size_t count)
{
    index = index >= 0 ? index : (stack->count + index);

    size_t stack_count = count + stack->count;

    if (stack_count > stack->capacity)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity = next_power_of_two(stack_count)));

    memmove(stack->items + index + count, stack->items + index, sizeof *stack->items * (stack->count - index));
    memcpy(stack->items + index, items, sizeof *items * count);

    stack->count = stack_count;
}
