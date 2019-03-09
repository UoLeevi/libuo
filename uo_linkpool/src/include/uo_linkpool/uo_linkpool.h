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

#define UO__LINKPOOL_DEFAULT_GROW       0x10

#define uo_link_type(type) \
    type ## _link

#define uo_linkpool_stack(type) \
    type ## _linkpool_stack

#define uo_linkpool_head(type) \
    type ## _linkpool_head

#define uo_linkpool_count(type) \
    type ## _linkpool_count

#define uo_linkpool_grow(type) \
    type ## _linkpool_grow

#define uo_linkpool_init(type) \
    type ## _linkpool_init

#define uo_linkpool_quit(type) \
    type ## _linkpool_quit

#define uo_linkpool_rent(type) \
    type ## _linkpool_rent

#define uo_linkpool_return(type) \
    type ## _linkpool_return

/**
 * @brief macro to define a linkpool for type
 * 
 * @param type  typedef'd name of the type to define the linkpool for
 */
#define uo_def_linkpool(type)                                                   \
\
uo_def_link(type);                                                              \
\
static _Thread_local uo_stack uo_linkpool_stack(type);                          \
static _Thread_local uo_linklist uo_linkpool_head(type);                        \
static _Thread_local size_t uo_linkpool_count(type);                            \
\
/* @brief allocate 'count' new nodes to the linkpool                         */ \
static void uo_linkpool_grow(type)(                                             \
    size_t count)                                                               \
{                                                                               \
    assert(uo_linkpool_stack(type).items);                                      \
\
    uo_linkpool_count(type) += count;                                           \
    uo_link_type(type) *items = calloc(count, sizeof *items);                   \
    uo_stack_push(&uo_linkpool_stack(type), items);                             \
\
    for (size_t i = 0; i < count; ++i)                                          \
        uo_linklist_insert_after(&uo_linkpool_head(type), items + i);           \
}                                                                               \
\
/* @brief initialize the linkpool for current thread                         */ \
static bool uo_linkpool_init(type)(void)                                        \
{                                                                               \
    if (!uo_linkpool_stack(type).items)                                         \
        uo_stack_create_at(&uo_linkpool_stack(type), 0);                        \
\
    return true;                                                                \
}                                                                               \
\
/* @brief free the linkpool resources held by current thread                 */ \
static void uo_linkpool_quit(type)(void)                                        \
{                                                                               \
    uo_linkpool_count(type) = 0;                                                \
    size_t count = uo_stack_count(&uo_linkpool_stack(type));                    \
\
    for (size_t i = 0; i < count; ++i)                                          \
        free(uo_stack_pop(&uo_linkpool_stack(type)));                           \
\
    uo_stack_destroy_at(&uo_linkpool_stack(type));                              \
    uo_linkpool_stack(type).items = NULL;                                       \
}                                                                               \
\
/* @brief take an linked list item off the linkpool                          */ \
static uo_link_type(type) *uo_linkpool_rent(type)(void)                         \
{                                                                               \
    if (!uo_linkpool_count(type))                                               \
        uo_linkpool_grow(type)(UO__LINKPOOL_DEFAULT_GROW);                      \
\
    --uo_linkpool_count(type);                                                  \
    uo_linklist *link = uo_linklist_next(&uo_linkpool_head(type));              \
    uo_linklist_remove(link);                                                   \
\
    return (uo_link_type(type) *)link;                                          \
}                                                                               \
\
/* @brief return an linked list item back to the linkpool                    */ \
static inline void uo_linkpool_return(type)(                                    \
    uo_link_type(type) *link)                                                   \
{                                                                               \
    ++uo_linkpool_count(type);                                                  \
    uo_linklist_insert_after(&uo_linkpool_head(type), (uo_linklist *)link);     \
}

#ifdef __cplusplus
}
#endif

#endif