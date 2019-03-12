#ifndef UO_STACK_H
#define UO_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#define UO_STACK_MIN_CAPACITY 0x4

/**
 * @brief stack data structure that supports automatic resizing
 * 
 * For convenience, uo_stack has also support for accessing and inserting stack items by using an index.
 */
typedef struct uo_stack
{
    void **items;
    size_t count;
    size_t capacity;
} uo_stack;

// ideally this would be defined in some other header or library
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

/**
 * @brief create an instance of uo_stack at specific memory location
 * 
 * @param stack             pointer to memory location where to create the uo_stack at
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_stack is full
 */
static inline void uo_stack_create_at(
    uo_stack *stack,
    size_t initial_capacity)
{
    size_t capacity = stack->capacity = initial_capacity >= UO_STACK_MIN_CAPACITY
        ? next_power_of_two(initial_capacity)
        : UO_STACK_MIN_CAPACITY;

    stack->count = 0;
    stack->items = malloc(sizeof *stack->items * capacity);
}

/**
 * @brief create an instance of uo_stack
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_stack is full
 * @return uo_stack *  created uo_stack instance
 */
static inline uo_stack *uo_stack_create(
    size_t initial_capacity)
{
    uo_stack *stack = malloc(sizeof *stack);
    uo_stack_create_at(stack, initial_capacity);
    return stack;
}

/**
 * @brief free resources used by an uo_stack instance but do not free the uo_stack pointer itself
 * 
 */
static inline void uo_stack_destroy_at(
    uo_stack *stack)
{
    free(stack->items);
}

/**
 * @brief free resources used by an uo_stack instance
 * 
 */
static inline void uo_stack_destroy(
    uo_stack *stack)
{
    uo_stack_destroy_at(stack);
    free(stack);
}

/**
 * @brief get the number of items on the stack
 * 
 */
static inline size_t uo_stack_count(
    uo_stack *stack)
{
    return stack->count;
}

/**
 * @brief push an item on to the stack
 * 
 */
static inline void uo_stack_push(
    uo_stack *stack,
    const void *item)
{
    size_t count = stack->count++;

    if (count == stack->capacity)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity <<= 1));

    stack->items[count] = (void *)item;
}

/**
 * @brief push an array of pointers on to the stack
 * 
 */
static inline void uo_stack_push_arr(
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

/**
 * @brief pop the item on top of the stack
 * 
 * The stack is required to have count greater than zero when this function is called.
 * 
 * @return void *   the item that was removed
 */
static inline void *uo_stack_pop(
    uo_stack *stack)
{
    assert(stack->count);

    size_t count = --stack->count;
    void *item = stack->items[count];

    if (count == stack->capacity >> 3 && count >= UO_STACK_MIN_CAPACITY)
        stack->items = realloc(stack->items, sizeof *stack->items * (stack->capacity >>= 1));

    return item;
}

/**
 * @brief get the item on top of the stack
 * 
 */
static inline void *uo_stack_peek(
    uo_stack *stack)
{
    return stack->items[stack->count - 1];
}


/**
 * @brief get an item from the stack by index
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
static inline void *uo_stack_index(
    uo_stack *stack,
    int index)
{
    return stack->items[index >= 0 ? index : (stack->count + index)];
}

/**
 * @brief insert an item on to stack at specified index
 * 
 * Note that this function is implemented by moving the array items after the index by one
 * which makes this function slower than uo_stack_push.
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
static inline void uo_stack_insert(
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

/**
 * @brief insert an array of pointers on to the stack at specified index
 * 
 * Note that this function is implemented by moving the array items after the index by the number of added items
 * which makes this function slower than uo_stack_push_arr.
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
static inline void uo_stack_insert_arr(
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

#ifdef __cplusplus
}
#endif

#endif
