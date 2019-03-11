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

#define uo_linkpool_stack(type) \
    type ## _linkpool_stack

#define uo_linkpool_head(type) \
    type ## _linkpool_head

#define uo_linkpool_is_init(type) \
    type ## _linkpool_is_init

#define uo_linkpool_init(type) \
    type ## _linkpool_init

#define uo_linkpool_quit(type) \
    type ## _linkpool_quit

#define uo_linkpool_is_empty(type) \
    type ## _linkpool_is_empty

#define uo_linkpool_grow(type) \
    type ## _linkpool_grow

#define uo_linkpool_rent(type) \
    type ## _linkpool_rent

#define uo_linkpool_return(type) \
    type ## _linkpool_return

/**
 * @brief macro to define a linkpool for type
 * 
 * @param type  typedef'd name of the type to define the linkpool for
 */
#define uo_def_linkpool(type)                                                                 \
\
uo_def_link(type);                                                                            \
\
static _Thread_local uo_stack uo_linkpool_stack(type);                                        \
static _Thread_local uo_linklist uo_linkpool_head(type);                                      \
\
/* @brief test if the linkpool has been initialized for current thread                     */ \
static inline bool uo_linkpool_is_init(type)(void)                                            \
{                                                                                             \
    return uo_linklist_is_linked(&uo_linkpool_head(type));                                    \
}                                                                                             \
\
/* @brief initialize the linkpool for current thread                                       */ \
static bool uo_linkpool_init(type)(void)                                                      \
{                                                                                             \
    if (uo_linkpool_head(type).next)                                                          \
        return true;                                                                          \
\
    uo_linklist_selflink(&uo_linkpool_head(type));                                            \
    uo_stack_create_at(&uo_linkpool_stack(type), 0);                                          \
    return true;                                                                              \
}                                                                                             \
\
/* @brief free the linkpool resources held by current thread                               */ \
static void uo_linkpool_quit(type)(void)                                                      \
{                                                                                             \
    if (!uo_linkpool_head(type).next)                                                         \
        return;                                                                               \
\
    size_t count = uo_stack_count(&uo_linkpool_stack(type));                                  \
\
    for (size_t i = 0; i < count; ++i)                                                        \
        free(uo_stack_pop(&uo_linkpool_stack(type)));                                         \
\
    uo_linklist_reset(&uo_linkpool_head(type));                                               \
    uo_stack_destroy_at(&uo_linkpool_stack(type));                                            \
}                                                                                             \
\
/* @brief test if the linkpool is empty for current thread                                 */ \
static inline bool uo_linkpool_is_empty(type)(void)                                           \
{                                                                                             \
    return uo_linklist_is_empty(&uo_linkpool_head(type));                                     \
}                                                                                             \
\
/* @brief allocate new nodes to the linkpool                                               */ \
static void uo_linkpool_grow(type)(                                                           \
    size_t count)                                                                             \
{                                                                                             \
    assert(uo_linkpool_is_init(type));                                                        \
\
    uo_link_type(type) *items = calloc(count, sizeof *items);                                 \
    uo_stack_push(&uo_linkpool_stack(type), items);                                           \
\
    for (size_t i = 0; i < count; ++i)                                                        \
        uo_linklist_link(&uo_linkpool_head(type), (uo_linklist *)(items + i));                \
}                                                                                             \
\
/* @brief take an linked list item off the linkpool                                        */ \
static inline uo_link_type(type) *uo_linkpool_rent(type)(void)                                \
{                                                                                             \
    if (uo_linkpool_is_empty(type))                                                           \
        uo_linkpool_grow(type)(UO__LINKPOOL_DEFAULT_GROW);                                    \
\
    uo_linklist *link = uo_linkpool_head(type).prev;                                          \
    uo_linklist_unlink(link);                                                                 \
\
    return (uo_link_type(type) *)link;                                                        \
}                                                                                             \
\
/* @brief return an linked list item back to the linkpool                                  */ \
static inline void uo_linkpool_return(type)(                                                  \
    uo_link_type(type) *link)                                                                 \
{                                                                                             \
    uo_linklist_link(&uo_linkpool_head(type), (uo_linklist *)link);                           \
}

#ifdef __cplusplus
}
#endif

#endif