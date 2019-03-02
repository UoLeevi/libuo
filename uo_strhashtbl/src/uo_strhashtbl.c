#include "uo_strhashtbl.h"
#include "uo_linklist.h"

#include <pthread.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define UO_STRHASHTBL_MIN_CAPACITY 0x10
#define UO_STRHASHTBL_TAG_REMOVED ((uintptr_t)~0)

struct uo_strhashtbl_item
{
    uo_linklist link;
    uo_strkvp strkvp;
};

struct uo_strhashtbl
{
    uo_linklist head;
    size_t count;
    size_t removed_count;
    size_t capacity;
    struct uo_strhashtbl_item *items;
    pthread_mutex_t mtx;
};

// http://www.cse.yorku.ca/~oz/hash.html#djb2
unsigned long uo_strhashtbl_hash(
    const unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    for (int i = 0; (c = *str++) && i < 0x10; ++i)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uo_strhashtbl *uo_strhashtbl_create(
    size_t initial_capacity)
{
    size_t capacity = UO_STRHASHTBL_MIN_CAPACITY;
    while (capacity <= initial_capacity)
        capacity <<= 1;

    uo_strhashtbl *strhashtbl = calloc(1, sizeof *strhashtbl);
    strhashtbl->items = calloc(strhashtbl->capacity = capacity, sizeof *strhashtbl->items);

    pthread_mutex_init(&strhashtbl->mtx, 0);

    return strhashtbl;
}

void uo_strhashtbl_destroy(
    uo_strhashtbl *strhashtbl)
{
    pthread_mutex_destroy(&strhashtbl->mtx);

    free(strhashtbl->items);
    free(strhashtbl);
}

size_t uo_strhashtbl_get_count(
    uo_strhashtbl *strhashtbl)
{
    return strhashtbl->count;
}

static struct uo_strhashtbl_item *uo_strhashtbl_find_item(
    const struct uo_strhashtbl_item *items,
    size_t capacity, 
    const char *key)
{
    const unsigned long mask = capacity - 1;
    unsigned long h = uo_strhashtbl_hash((const unsigned char *)key) & mask;
    const struct uo_strhashtbl_item *item = items + h;

    while (item->strkvp.key && strcmp(item->strkvp.key, key) || (uintptr_t)item->strkvp.value == UO_STRHASHTBL_TAG_REMOVED)
        item = items + (++h & mask);

    return (struct uo_strhashtbl_item *)item;
}

void *uo_strhashtbl_find(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    if (!key)
        return NULL;

    pthread_mutex_lock(&strhashtbl->mtx);

    void *value = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key)->strkvp.value;

    pthread_mutex_unlock(&strhashtbl->mtx);

    return value;
}

uo_strkvp uo_strhashtbl_find_next_strkvp(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    if (!strhashtbl->count)
        return (uo_strkvp) { .key = NULL, .value = NULL };

    if (key)
    {
        struct uo_strhashtbl_item *item = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key);

        if (!item)
            return (uo_strkvp) { .key = NULL, .value = NULL };

        item = uo_linklist_next(item) == strhashtbl
            ? uo_linklist_next(uo_linklist_next(item))
            : uo_linklist_next(item);

        return item->strkvp;
    }

    struct uo_strhashtbl_item *item = uo_linklist_next(strhashtbl);

    return item->strkvp;
}

static void uo_strhashtbl_resize(
    uo_strhashtbl *strhashtbl,
    const size_t capacity)
{
    strhashtbl->capacity = capacity;
    struct uo_strhashtbl_item *new_items = calloc(capacity, sizeof *strhashtbl->items);

    struct uo_strhashtbl_item *item = uo_linklist_next(strhashtbl);
    uo_linklist_remove(strhashtbl);

    struct uo_strhashtbl_item *new_item = uo_strhashtbl_find_item(new_items, capacity, item->strkvp.key);

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

    pthread_mutex_lock(&strhashtbl->mtx);

    struct uo_strhashtbl_item *item = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key);

    item->strkvp.value = (void *)value;

    if (item->strkvp.key)
    {
        pthread_mutex_unlock(&strhashtbl->mtx);
        return;
    }

    item->strkvp.key = key;

    if (!uo_linklist_prev(strhashtbl))
        uo_linklist_insert_after(strhashtbl, item);
        
    uo_linklist_insert_before(strhashtbl, item);

    if (++strhashtbl->count == strhashtbl->capacity >> 1)
    {
        uo_strhashtbl_resize(strhashtbl, strhashtbl->capacity << 1);
        strhashtbl->removed_count = 0;
    }

    pthread_mutex_unlock(&strhashtbl->mtx);
}

void *uo_strhashtbl_remove(
    uo_strhashtbl *strhashtbl, 
    const char *key)
{
    if (!key)
        return NULL;

    pthread_mutex_lock(&strhashtbl->mtx);

    struct uo_strhashtbl_item *item = uo_strhashtbl_find_item(strhashtbl->items, strhashtbl->capacity, key);

    if (!item->strkvp.key)
    {
        pthread_mutex_unlock(&strhashtbl->mtx);
        return NULL;
    }

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

    pthread_mutex_unlock(&strhashtbl->mtx);

    return value;
}

