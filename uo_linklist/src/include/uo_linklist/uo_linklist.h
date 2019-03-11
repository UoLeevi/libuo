#ifndef UO_LINKLIST_H
#define UO_LINKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

/**
 * @brief uo_linklist is structure for chaining structs in a doubly linked list.
 * 
 * When used as a linked list, uo_linklist list consists of:
 *  - an uo_linklist head which initially should link to itself (initialize using uo_linklist_selflink)
 *  - zero or more uo_linklist links which should be zero-filled before they were inserted to the list
 * 
 * uo_linklist list, that has a properly initialized head, is always a circular doubly linked list such:
 *  - for the head and all uo_linklist links, link == link->next->prev && link == link->prev->next
 *  - this also means that following is never true: link->next == NULL || link->prev == NULL
 */
typedef struct uo_linklist
{
    struct uo_linklist *next;
    struct uo_linklist *prev;
} uo_linklist;

#define uo_link_type(type) \
    type ## _link

#define uo_link_next(type) \
    type ## _link_next

#define uo_link_prev(type) \
    type ## _link_prev

#define uo_get_link(type) \
    type ## _get_link

#define uo_def_link(type)                                                                     \
\
/* @brief typed linked list entry                                                          */ \
typedef struct uo_link_type(type)                                                             \
{                                                                                             \
    uo_linklist link;                                                                         \
    type item;                                                                                \
} uo_link_type(type);                                                                         \
\
static inline uo_link_type(type) *uo_link_next(type)(                                         \
    uo_link_type(type) *link)                                                                 \
{                                                                                             \
    return (uo_link_type(type) *)uo_linklist_next(link);                                      \
}                                                                                             \
\
static inline uo_link_type(type) *uo_link_prev(type)(                                         \
    uo_link_type(type) *link)                                                                 \
{                                                                                             \
    return (uo_link_type(type) *)uo_linklist_prev(link);                                      \
}                                                                                             \
\
static inline uo_link_type(type) *uo_get_link(type)(                                          \
    type *item)                                                                               \
{                                                                                             \
    return (uo_link_type(type) *)((char *)(void *)item - offsetof(uo_link_type(type), item)); \
}

/**
 * @brief get next link in the linklist
 * 
 */
#define uo_linklist_next(link) \
    (((uo_linklist *)(link))->next)

/**
 * @brief get previous link in the linklist
 * 
 */
#define uo_linklist_prev(link) \
    (((uo_linklist *)(link))->prev)

/**
 * @brief initialize a head
 * 
 */
#define uo_linklist_selflink(head) \
    uo__linklist_selflink((uo_linklist *)(head))

/**
 * @brief zero-fill a link
 * 
 */
#define uo_linklist_reset(link) \
    uo__linklist_reset((uo_linklist *)(link))

/**
 * @brief test if list is empty
 * 
 */
#define uo_linklist_is_empty(head) \
    uo__linklist_is_empty((uo_linklist *)(head))

/**
 * @brief test if link is linked to a list
 * 
 */
#define uo_linklist_is_linked(link) \
    uo__linklist_is_linked((uo_linklist *)(link))

/**
 * @brief insert link before position marked by another link
 * 
 */
#define uo_linklist_link(pos, link) \
    uo__linklist_link((uo_linklist *)(pos), (uo_linklist *)(link))

/**
 * @brief remove link from linklist
 * 
 */
#define uo_linklist_unlink(link) \
    uo__linklist_unlink((uo_linklist *)(link))

static inline void uo__linklist_selflink(
    uo_linklist *head)
{
    head->next = head->prev = head;
}

static inline void uo__linklist_reset(
    uo_linklist *link)
{
    link->next = link->prev = NULL;
}

static inline bool uo__linklist_is_empty(
    uo_linklist *head)
{
    return head == head->next;
}

static inline bool uo__linklist_is_linked(
    uo_linklist *link)
{
    return link->next != NULL;
}

static inline void uo__linklist_link(
    uo_linklist *pos,
    uo_linklist *link)
{
    assert(!uo__linklist_is_linked(link));

    (link->prev = pos->prev)->next = link;
    (link->next = pos)->prev = link;
}

static inline void uo__linklist_unlink(
    uo_linklist *link)
{
    link->next->prev = link->prev;
    link->prev->next = link->next;

    uo_linklist_reset(link);
}

#ifdef __cplusplus
}
#endif

#endif
