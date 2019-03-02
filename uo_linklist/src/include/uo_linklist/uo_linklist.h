#ifndef UO_LINKLIST_H
#define UO_LINKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>

/**
 * @brief list for linking memory addresses
 * 
 * Use this struct as the first member of another struct to make the parent 
 * struct a "link" that can be used in a linklist. To access the parent struct, 
 * just convert the uo_linklist pointer to a pointer of the parent type.
 * 
 * All uo_linklist structs should be zero initialized before use.
 * uo_linklist next and prev are NULL when they do point to another link.
 * i.e. Empty linklist has next and prev set to NULL.
 * 
 * Circular linklists are permitted but linklist should never be inserted to itself.
 */
typedef struct uo_linklist
{
    struct uo_linklist *next;
    struct uo_linklist *prev;
} uo_linklist;

/**
 * @brief get next link
 * 
 */
#define uo_linklist_next(link) \
    ((void *)(((uo_linklist *)(link))->next))

/**
 * @brief get previous link
 * 
 */
#define uo_linklist_prev(link) \
    ((void *)(((uo_linklist *)(link))->prev))

/**
 * @brief zero initialize a link
 * 
 */
#define uo_linklist_reset(link) \
    uo__linklist_reset((uo_linklist *)(link))

/**
 * @brief insert link after position marked by another link
 * 
 */
#define uo_linklist_insert_before(pos, link) \
    uo__linklist_insert_before((uo_linklist *)(pos), (uo_linklist *)(link))

/**
 * @brief insert link before position marked by another link
 * 
 */
#define uo_linklist_insert_after(pos, link) \
    uo__linklist_insert_after((uo_linklist *)(pos), (uo_linklist *)(link))

/**
 * @brief remove link from linklist
 * 
 */
#define uo_linklist_remove(link) \
    uo__linklist_remove((uo_linklist *)(link))

static inline void uo__linklist_reset(
    uo_linklist *link)
{
    link->next = link->prev = NULL;
}

static inline void uo__linklist_insert_before(
    uo_linklist *pos,
    uo_linklist *link)
{
    assert(!link->next);

    link->next = pos;

    if (pos->prev)
    {
        assert(!link->prev);

        pos->prev->next = link;
        link->prev = pos->prev;
    }

    pos->prev = link;
}

static inline void uo__linklist_insert_after(
    uo_linklist *pos,
    uo_linklist *link)
{
    assert(!link->prev);

    link->prev = pos;

    if (pos->next)
    {
        assert(!link->next);

        pos->next->prev = link;
        link->next = pos->next;
    }

    pos->next = link;
}

static inline void uo__linklist_remove(
    uo_linklist *link)
{
    uo_linklist *link_next = link->next;
    uo_linklist *link_prev = link->prev;

    if (link_next)
        link_next->prev = link_next != link_prev ? link_prev : NULL;

    if (link_prev)
        link_prev->next = link_prev != link_next ? link_next : NULL;

    uo_linklist_reset(link);
}

#ifdef __cplusplus
}
#endif

#endif
