#include "uo_strhashtbl.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define UO_STRHASHTBL_MIN_CAPACITY 0x10
#define UO_STRHASHTBL_TAG_REMOVED ((uintptr_t)~0)

struct uo_strhashtbl
{
    uo_linklist head;
    size_t count;
    size_t removed_count;
    size_t capacity;
    uo_strkvp_link *links;
};

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

// http://www.cse.yorku.ca/~oz/hash.html#djb2
static unsigned long uo_strhashtbl_hash(
    const unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    for (int i = 0; (c = *str++) && i < 0x10; ++i)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void uo_strhashtbl_create_at(
    uo_strhashtbl *strhashtbl,
    size_t initial_capacity)
{
    size_t capacity = initial_capacity >= UO_STRHASHTBL_MIN_CAPACITY
        ? next_power_of_two(initial_capacity)
        : UO_STRHASHTBL_MIN_CAPACITY;

    strhashtbl->count = 0;
    strhashtbl->removed_count = 0;
    strhashtbl->links = calloc(strhashtbl->capacity = capacity, sizeof *strhashtbl->links);

    uo_linklist_selflink(strhashtbl);
}

uo_strhashtbl *uo_strhashtbl_create(
    size_t initial_capacity)
{
    uo_strhashtbl *strhashtbl = malloc(sizeof *strhashtbl);
    uo_strhashtbl_create_at(strhashtbl, initial_capacity);
    return strhashtbl;
}

void uo_strhashtbl_destroy_at(
    uo_strhashtbl *strhashtbl)
{
    free(strhashtbl->links);
}

void uo_strhashtbl_destroy(
    uo_strhashtbl *strhashtbl)
{
    free(strhashtbl->links);
    free(strhashtbl);
}

inline size_t uo_strhashtbl_count(
    uo_strhashtbl *strhashtbl)
{
    return strhashtbl->count;
}

static uo_strkvp_link *uo_strhashtbl_find_link(
    const uo_strkvp_link *links,
    size_t capacity,
    const char *key)
{
    const unsigned long mask = capacity - 1;
    unsigned long h = uo_strhashtbl_hash((const unsigned char *)key) & mask;
    const uo_strkvp_link *link = links + h;

    while (link->item.key && strcmp(link->item.key, key) || (uintptr_t)link->item.value == UO_STRHASHTBL_TAG_REMOVED)
        link = links + (++h & mask);

    return (uo_strkvp_link *)link;
}

inline uo_strkvp_link *uo_strhashtbl_list(
    uo_strhashtbl *strhashtbl)
{
    return (uo_strkvp_link *)&strhashtbl->head;
}

static void uo_strhashtbl_resize(
    uo_strhashtbl *strhashtbl,
    const size_t capacity)
{
    strhashtbl->capacity = capacity;
    uo_strkvp_link *new_links = calloc(capacity, sizeof *strhashtbl->links);

    uo_linklist head = strhashtbl->head;

    uo_strkvp_link *link = (uo_strkvp_link *)&head;
    uo_strkvp_link *new_link;

    uo_linklist_unlink(strhashtbl);
    uo_linklist_selflink(strhashtbl);

    size_t count = strhashtbl->count;

    for (size_t i = 0; i < count; ++i)
    {
        link = uo_strkvp_link_next(link);
        new_link = uo_strhashtbl_find_link(new_links, capacity, link->item.key);
        new_link->item = link->item;
        uo_linklist_link(strhashtbl, new_link);
    }

    free(strhashtbl->links);
    strhashtbl->links = new_links;
}

void *uo_strhashtbl_get(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    return key 
        ? uo_strhashtbl_find_link(strhashtbl->links, strhashtbl->capacity, key)->item.value
        : NULL;
}

void uo_strhashtbl_set(
    uo_strhashtbl *strhashtbl, 
    const char *key, 
    const void *value)
{
    if (!key)
        return;

    uo_strkvp_link *link = uo_strhashtbl_find_link(strhashtbl->links, strhashtbl->capacity, key);
    link->item.value = (void *)value;

    if (link->item.key)
        return;

    link->item.key = key;

    uo_linklist_link(strhashtbl, link);

    if (++strhashtbl->count == strhashtbl->capacity >> 1)
    {
        uo_strhashtbl_resize(strhashtbl, strhashtbl->capacity << 1);
        strhashtbl->removed_count = 0;
    }
}

void *uo_strhashtbl_remove(
    uo_strhashtbl *strhashtbl, 
    const char *key)
{
    if (!key)
        return NULL;

    uo_strkvp_link *link = uo_strhashtbl_find_link(strhashtbl->links, strhashtbl->capacity, key);

    if (!link->item.key)
        return NULL;

    void *value = link->item.value;
    link->item.key = NULL;
    link->item.value = (void *)UO_STRHASHTBL_TAG_REMOVED;

    uo_linklist_unlink(link);

    size_t removed_count = ++strhashtbl->removed_count;
    size_t count = --strhashtbl->count;

    if (removed_count >= strhashtbl->capacity >> 2)
    {
        uo_strhashtbl_resize(strhashtbl, strhashtbl->capacity);
        strhashtbl->removed_count = 0;
    }

    if ((count == strhashtbl->capacity >> 3) >= UO_STRHASHTBL_MIN_CAPACITY)
    {
        uo_strhashtbl_resize(strhashtbl, strhashtbl->capacity >> 1);
        strhashtbl->removed_count = 0;
    }

    return value;
}
