#ifndef UO_linkpool_H
#define UO_linkpool_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_linklist.h"
#include "uo_queue.h"
#include "uo_stack.h"
#include "uo_macro.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#define UO__LINKPOOL_DEFAULT_GROW 0x20

#define uo__linkpool_queue(prefix) \
    prefix ## _linkpool_queue

#define uo__linkpool_stack(prefix) \
    prefix ## _linkpool_stack

#define uo__linkpool_head(prefix) \
    prefix ## _linkpool_head

#define uo__linkpool_is_init_f(prefix) \
    prefix ## _linkpool_is_init

#define uo__linkpool_init_f(prefix) \
    prefix ## _linkpool_init

#define uo__linkpool_quit_f(prefix) \
    prefix ## _linkpool_quit

#define uo__linkpool_is_thrd_init_f(prefix) \
    prefix ## _linkpool_is_thrd_init

#define uo__linkpool_thrd_init_f(prefix) \
    prefix ## _linkpool_thrd_init

#define uo__linkpool_thrd_quit_f(prefix) \
    prefix ## _linkpool_thrd_quit

#define uo__linkpool_is_empty_f(prefix) \
    prefix ## _linkpool_is_empty

#define uo__linkpool_grow_f(prefix) \
    prefix ## _linkpool_grow

#define uo__linkpool_rent_f(prefix) \
    prefix ## _linkpool_rent

#define uo__linkpool_return_f(prefix) \
    prefix ## _linkpool_return

/**
 * @brief macro to declare a linkpool for type
 * 
 * @param prefix    prefix for the new linkpool type to be declared
 * @param type      type to declare the linkpool for
 */
#define uo_decl_linkpool(prefix, type)                                                        \
                                                                                              \
extern uo_queue *uo__linkpool_queue(prefix);                                                  \
extern _Thread_local uo_stack *uo__linkpool_stack(prefix);                                    \
extern _Thread_local uo_linklist uo__linkpool_head(prefix);                                   \
                                                                                              \
/* @brief test if the linkpool has been initialized                                        */ \
static inline bool uo__linkpool_is_init_f(prefix)(void)                                       \
{                                                                                             \
    return uo__linkpool_queue(prefix) != NULL;                                                \
}                                                                                             \
                                                                                              \
/* @brief initialize the linkpool functionality                                            */ \
bool uo__linkpool_init_f(prefix)(void);                                                       \
                                                                                              \
/* @brief test if the linkpool has been initialized for current thread                     */ \
static inline bool uo__linkpool_is_thrd_init_f(prefix)(void)                                  \
{                                                                                             \
    return uo_linklist_is_linked(&uo__linkpool_head(prefix));                                 \
}                                                                                             \
                                                                                              \
/* @brief initialize the linkpool for current thread                                       */ \
bool uo__linkpool_thrd_init_f(prefix)(void);                                                  \
                                                                                              \
/* @brief free the linkpool resources held by current thread                               */ \
void uo__linkpool_thrd_quit_f(prefix)(void);                                                  \
                                                                                              \
/* @brief test if the linkpool is empty for current thread                                 */ \
static inline bool uo__linkpool_is_empty_f(prefix)(void)                                      \
{                                                                                             \
    return uo_linklist_is_empty(&uo__linkpool_head(prefix));                                  \
}                                                                                             \
                                                                                              \
/* @brief allocate new nodes to the linkpool                                               */ \
void uo__linkpool_grow_f(prefix)(                                                             \
    size_t count);                                                                            \
                                                                                              \
/* @brief take an linked list item off the linkpool                                        */ \
uo__linklist_type(prefix) *uo__linkpool_rent_f(prefix)(void);                                 \
                                                                                              \
/* @brief return an linked list item back to the linkpool                                  */ \
static inline void uo__linkpool_return_f(prefix)(                                             \
    uo__linklist_type(prefix) *link)                                                          \
{                                                                                             \
    uo_linklist_link(&uo__linkpool_head(prefix), (uo_linklist *)link);                        \
}

#define uo_impl_linkpool(prefix, type)                                                        \
                                                                                              \
uo_queue *uo__linkpool_queue(prefix);                                                         \
_Thread_local uo_stack *uo__linkpool_stack(prefix);                                           \
_Thread_local uo_linklist uo__linkpool_head(prefix);                                          \
                                                                                              \
static void uo__linkpool_quit_f(prefix)(void)                                                 \
{                                                                                             \
    if (!uo__linkpool_is_init_f(prefix)())                                                    \
        return;                                                                               \
                                                                                              \
    uo_stack *stack;                                                                          \
    while (stack = uo_queue_dequeue(uo__linkpool_queue(prefix), false))                       \
    {                                                                                         \
        size_t count = uo_stack_count(stack);                                                 \
                                                                                              \
        for (size_t i = 0; i < count; ++i)                                                    \
            free(uo_stack_pop(stack));                                                        \
                                                                                              \
        uo_stack_destroy(stack);                                                              \
    }                                                                                         \
                                                                                              \
    uo_queue_destroy(uo__linkpool_queue(prefix));                                             \
}                                                                                             \
                                                                                              \
bool uo__linkpool_init_f(prefix)()                                                            \
{                                                                                             \
    if (uo__linkpool_is_init_f(prefix)())                                                     \
        return true;                                                                          \
                                                                                              \
    uo__linkpool_queue(prefix) = uo_queue_create(0x10);                                       \
    atexit(uo__linkpool_quit_f(prefix));                                                      \
    return true;                                                                              \
}                                                                                             \
                                                                                              \
bool uo__linkpool_thrd_init_f(prefix)()                                                       \
{                                                                                             \
    if (uo__linkpool_is_thrd_init_f(prefix)())                                                \
        return true;                                                                          \
                                                                                              \
    uo_stack *stack = uo_queue_dequeue(uo__linkpool_queue(prefix), false);                    \
    uo__linkpool_stack(prefix) = stack ? stack : uo_stack_create(0);                          \
    uo_linklist_selflink(&uo__linkpool_head(prefix));                                         \
    return true;                                                                              \
}                                                                                             \
                                                                                              \
void uo__linkpool_thrd_quit_f(prefix)()                                                       \
{                                                                                             \
    if (!uo__linkpool_is_thrd_init_f(prefix)())                                               \
        return;                                                                               \
                                                                                              \
    uo_queue_enqueue(uo__linkpool_queue(prefix), uo__linkpool_stack(prefix), true);           \
    uo_linklist_reset(&uo__linkpool_head(prefix));                                            \
}                                                                                             \
                                                                                              \
void uo__linkpool_grow_f(prefix)(                                                             \
    size_t count)                                                                             \
{                                                                                             \
    assert(uo__linkpool_is_thrd_init_f(prefix));                                              \
                                                                                              \
    uo__linklist_type(prefix) *items = calloc(count, sizeof *items);                          \
    uo_stack_push(uo__linkpool_stack(prefix), items);                                         \
                                                                                              \
    for (size_t i = 0; i < count; ++i)                                                        \
        uo_linklist_link(&uo__linkpool_head(prefix), (uo_linklist *)(items + i));             \
}                                                                                             \
                                                                                              \
uo__linklist_type(prefix) *uo__linkpool_rent_f(prefix)()                                      \
{                                                                                             \
    if (uo__linkpool_is_empty_f(prefix))                                                      \
        uo__linkpool_grow_f(prefix)(UO__LINKPOOL_DEFAULT_GROW);                               \
                                                                                              \
    uo_linklist *link = uo__linkpool_head(prefix).prev;                                       \
    uo_linklist_unlink(link);                                                                 \
                                                                                              \
    return (uo__linklist_type(prefix) *)link;                                                 \
}

#ifdef __cplusplus
}
#endif

#endif
