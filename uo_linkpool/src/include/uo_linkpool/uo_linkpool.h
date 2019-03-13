#ifndef UO_linkpool_H
#define UO_linkpool_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_linklist.h"
#include "uo_stack.h"
#include "uo_macro.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#define UO__LINKPOOL_DEFAULT_GROW 0x10

#define uo__linkpool_stack(type) \
    type ## _linkpool_stack

#define uo__linkpool_head(type) \
    type ## _linkpool_head

#define uo__linkpool_is_init_f(type) \
    type ## _linkpool_is_init

#define uo__linkpool_init_f(type) \
    type ## _linkpool_init

#define uo__linkpool_quit_f(type) \
    type ## _linkpool_quit

#define uo__linkpool_is_empty_f(type) \
    type ## _linkpool_is_empty

#define uo__linkpool_grow_f(type) \
    type ## _linkpool_grow

#define uo__linkpool_rent_f(type) \
    type ## _linkpool_rent

#define uo__linkpool_return_f(type) \
    type ## _linkpool_return

/**
 * @brief macro to define a linkpool for type
 * 
 * @param type  typedef'd name of the type to define the linkpool for
 */
#define uo_def_linkpool(type)                                                                 \
\
uo_def_linklist(type);                                                                        \
\
static _Thread_local uo_stack uo__linkpool_stack(type);                                       \
static _Thread_local uo_linklist uo__linkpool_head(type);                                     \
\
/* @brief test if the linkpool has been initialized for current thread                     */ \
static inline bool uo__linkpool_is_init_f(type)(void)                                         \
{                                                                                             \
    return uo_linklist_is_linked(&uo__linkpool_head(type));                                   \
}                                                                                             \
\
/* @brief initialize the linkpool for current thread                                       */ \
static bool uo__linkpool_init_f(type)(void)                                                   \
{                                                                                             \
    if (uo__linkpool_head(type).next)                                                         \
        return true;                                                                          \
\
    uo_linklist_selflink(&uo__linkpool_head(type));                                           \
    uo_stack_create_at(&uo__linkpool_stack(type), 0);                                         \
    return true;                                                                              \
}                                                                                             \
\
/* @brief free the linkpool resources held by current thread                               */ \
static void uo__linkpool_quit_f(type)(void)                                                   \
{                                                                                             \
    if (!uo__linkpool_head(type).next)                                                        \
        return;                                                                               \
\
    size_t count = uo_stack_count(&uo__linkpool_stack(type));                                 \
\
    for (size_t i = 0; i < count; ++i)                                                        \
        free(uo_stack_pop(&uo__linkpool_stack(type)));                                        \
\
    uo_linklist_reset(&uo__linkpool_head(type));                                              \
    uo_stack_destroy_at(&uo__linkpool_stack(type));                                           \
}                                                                                             \
\
/* @brief test if the linkpool is empty for current thread                                 */ \
static inline bool uo__linkpool_is_empty_f(type)(void)                                        \
{                                                                                             \
    return uo_linklist_is_empty(&uo__linkpool_head(type));                                    \
}                                                                                             \
\
/* @brief allocate new nodes to the linkpool                                               */ \
static void uo__linkpool_grow_f(type)(                                                        \
    size_t count)                                                                             \
{                                                                                             \
    assert(uo__linkpool_is_init_f(type));                                                     \
\
    uo__linklist_type(type) *items = calloc(count, sizeof *items);                             \
    uo_stack_push(&uo__linkpool_stack(type), items);                                          \
\
    for (size_t i = 0; i < count; ++i)                                                        \
        uo_linklist_link(&uo__linkpool_head(type), (uo_linklist *)(items + i));               \
}                                                                                             \
\
/* @brief take an linked list item off the linkpool                                        */ \
static inline uo__linklist_type(type) *uo__linkpool_rent_f(type)(void)                        \
{                                                                                             \
    if (uo__linkpool_is_empty_f(type))                                                        \
        uo__linkpool_grow_f(type)(UO__LINKPOOL_DEFAULT_GROW);                                 \
\
    uo_linklist *link = uo__linkpool_head(type).prev;                                         \
    uo_linklist_unlink(link);                                                                 \
\
    return (uo__linklist_type(type) *)link;                                                   \
}                                                                                             \
\
/* @brief return an linked list item back to the linkpool                                  */ \
static inline void uo__linkpool_return_f(type)(                                               \
    uo__linklist_type(type) *link)                                                            \
{                                                                                             \
    uo_linklist_link(&uo__linkpool_head(type), (uo_linklist *)link);                          \
}

#ifdef __cplusplus
}
#endif

#endif