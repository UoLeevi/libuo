#ifndef UO_REFSTACK_H
#define UO_REFSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_refcount.h"
#include "uo_stack.h"

typedef struct uo_stack uo_refstack;

/**
 * @brief create an instance of uo_refstack
 * 
 * uo_refstack is used for grouping resources that should be finalized together.
 * The order of finalization for each of the grouped resource is reverse to the order they were added.
 * 
 * @return uo_refstack *  created uo_refstack instance
 */
static inline uo_refstack *uo_refstack_create(void)
{
    return uo_stack_create(0);
}

static inline void uo_refstack_create_at(
    uo_refstack *refstack)
{
    uo_stack_create_at(refstack, 0);
}

/**
 * @brief free resources used by an uo_refstack instance itself
 * 
 */
static inline void uo_refstack_destroy(
    uo_refstack *refstack)
{
    uo_stack_destroy(refstack);
}

static inline void uo_refstack_destroy_at(
    uo_refstack *refstack)
{
    uo_stack_destroy_at(refstack);
}

/**
 * @brief add resource to the uo_refstack stack
 * 
 * @param ptr           resource to be added to the refstack
 * @param finalizer     pointer to the finalizer function for the resource
 */
static inline void uo_refstack_push(
    uo_refstack *refstack,
    void *ptr,
    void (*finalizer)(void *))
{
    uo_refcount *refcount = uo_refcount_create(ptr, finalizer);
    uo_stack_push(refstack, refcount);
}

/**
 * @brief add refcount to the uo_refstack stack
 * 
 * uo_refcount_dec would calle on uo_refstack_finalize.
 */
static inline void uo_refstack_push_refcount(
    uo_refstack *refstack,
    uo_refcount *refcount)
{
    uo_stack_push(refstack, refcount);
}

/**
 * @brief finalize all the resources added to the uo_refstack instance
 * 
 */
static inline void uo_refstack_finalize(
    uo_refstack *refstack)
{
    size_t count = uo_stack_count(refstack);

    for (size_t i = 0; i < count; ++i)
        uo_refcount_dec(uo_stack_pop(refstack));
}

#ifdef __cplusplus
}
#endif

#endif