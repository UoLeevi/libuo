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
    uo_strkvplist *items;
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
    strhashtbl->items = calloc(strhashtbl->capacity = capacity, sizeof *strhashtbl->items);

    uo_linklist_reset(strhashtbl);
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
    free(strhashtbl->items);
}

void uo_strhashtbl_destroy(
    uo_strhashtbl *strhashtbl)
{
    free(strhashtbl->items);
    free(strhashtbl);
}

size_t uo_strhashtbl_count(
    uo_strhashtbl *strhashtbl)
{
    return strhashtbl->count;
}

static uo_strkvplist *uo_strhashtbl_find_item(
    const uo_strkvplist *items,
    size_t capacity, 
    const char *key)
{
    const unsigned long mask = capacity - 1;
    unsigned long h = uo_strhashtbl_hash((const unsigned char *)key) & mask;
    const uo_strkvplist *item = items + h;

    while (item->strkvp.key && strcmp(item->strkvp.key, key) || (uintptr_t)item->strkvp.value == UO_STRHASHTBL_TAG_REMOVED)
        item = items + (++h & mask);

    return (uo_strkvplist *)item;
}

void *uo_strhashtbl_find(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    return key ? uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key)->strkvp.value : NULL;
}

uo_strkvplist *uo_strhashtbl_list(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    return key 
        ? uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key)
        : uo_linklist_next(strhashtbl);
}

static void uo_strhashtbl_resize(
    uo_strhashtbl *strhashtbl,
    const size_t capacity)
{
    strhashtbl->capacity = capacity;
    uo_strkvplist *new_items = calloc(capacity, sizeof *strhashtbl->items);

    uo_strkvplist *item = uo_linklist_next(strhashtbl);
    uo_linklist_remove(strhashtbl);

    uo_strkvplist *new_item = uo_strhashtbl_find_item(new_items, capacity, item->strkvp.key);

    uo_linklist_insert_after(strhashtbl, new_item);
    uo_linklist_insert_before(strhashtbl, new_item);

    new_item->strkvp = item->strkvp;
    item = uo_linklist_next(item);

    size_t count = strhashtbl->count;

    for (size_t i = 1; i < count; ++i)
    {
        uo_linklist_insert_before(strhashtbl, uo_strhashtbl_find_item(new_items, capacity, item->strkvp.key));
        new_item = uo_linklist_next(new_item);

        new_item->strkvp = item->strkvp;
        item = uo_linklist_next(item);
    }

    free(strhashtbl->items);
    strhashtbl->items = new_items;
}

void uo_strhashtbl_insert(
    uo_strhashtbl *strhashtbl, 
    const char *key, 
    const void *value)
{
    if (!key)
        return;

    uo_strkvplist *item = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key);

    item->strkvp.value = (void *)value;

    if (item->strkvp.key)
        return;

    item->strkvp.key = key;

    if (!uo_linklist_prev(strhashtbl))
        uo_linklist_insert_after(strhashtbl, item);
        
    uo_linklist_insert_before(strhashtbl, item);

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

    uo_strkvplist *item = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key);

    if (!item->strkvp.key)
        return NULL;

    void *value = item->strkvp.value;
    item->strkvp.key = NULL;
    item->strkvp.value = (void *)UO_STRHASHTBL_TAG_REMOVED;

    uo_linklist_remove(item);

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
