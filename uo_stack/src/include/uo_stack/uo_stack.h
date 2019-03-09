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
 * @brief create an instance of uo_stack at specific memory location
 * 
 * uo_stack is a stack data structure that supports automatic resizing.
 * Unlike traditional stack data structure, uo_stack has also support for 
 * accessing and inserting stack items by index.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_stack is full
 */
void uo_stack_create_at(
    uo_stack *,
    size_t initial_capacity);

/**
 * @brief create an instance of uo_stack
 * 
 * uo_stack is a stack data structure that supports automatic resizing.
 * Unlike traditional stack data structure, uo_stack has also support for 
 * accessing and inserting stack items by index.
 * 
 * @param initial_capacity  minimum initial capacity, note that resize occures when uo_stack is full
 * @return uo_stack *  created uo_stack instance
 */
uo_stack *uo_stack_create(
    size_t initial_capacity);

/**
 * @brief free resources used by an uo_stack instance but do not free the uo_stack pointer itself
 * 
 */
void uo_stack_destroy_at(
    uo_stack *);

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
static inline size_t uo_stack_count(
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
 * @brief push an array of pointers on to the stack
 * 
 */
void uo_stack_push_arr(
    uo_stack *,
    void *const *items,
    size_t count);

/**
 * @brief pop the item on top of the stack
 * 
 * The stack is required to have count greater than zero when this function is called.
 * 
 * @return void *   the item that was removed
 */
void *uo_stack_pop(
    uo_stack *);

/**
 * @brief get the item on top of the stack
 * 
 */
void *uo_stack_peek(
    uo_stack *);

/**
 * @brief get an item from the stack by index
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
void *uo_stack_index(
    uo_stack *,
    int index);

/**
 * @brief insert an item on to stack at specified index
 * 
 * Note that this function is implemented by moving the array items after the index by one
 * which makes this function slower than uo_stack_push.
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
void uo_stack_insert(
    uo_stack *,
    int index,
    const void *item);

/**
 * @brief insert an array of pointers on to the stack at specified index
 * 
 * Note that this function is implemented by moving the array items after the index by the number of added items
 * which makes this function slower than uo_stack_push_arr.
 * 
 * @param index     use negative index to index starting from one past the last item of the stack
 */
void uo_stack_insert_arr(
    uo_stack *,
    int index,
    void *const *items,
    size_t count);

#ifdef __cplusplus
}
#endif

#endif