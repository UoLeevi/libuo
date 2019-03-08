#ifndef UO_STACK_H
#define UO_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct uo_stack 
{
    void **items;
    size_t count;
    size_t capacity;
} uo_stack;

/**
 * @brief create an instance of uo_stack
 * 
 * uo_stack is idiomatic stack data structure that supports automatic resizing.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_stack is full
 * @return uo_stack *  created uo_stack instance
 */
uo_stack *uo_stack_create(
    size_t initial_capacity);

/**
 * @brief free resources used by an uo_stack instance
 * 
 */
void uo_stack_destroy(
    uo_stack *);

/**
 * @brief get the number of items on the stack
 * 
 */
static inline size_t uo_stack_get_count(
    uo_stack *stack)
{
    return stack->count;
}

/**
 * @brief push an item on to the stack
 * 
 */
void uo_stack_push(
    uo_stack *,
    const void *item);

/**
 * @brief pop an item off the stack
 * 
 * The stack is required to have count greater than zero when this function is called.
 * 
 * @return void *   the item that was removed
 */
void *uo_stack_pop(
    uo_stack *);

#ifdef __cplusplus
}
#endif

#endif