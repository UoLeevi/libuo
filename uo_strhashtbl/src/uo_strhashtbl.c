#include "uo_strhashtbl.h"

#include <stdlib.h>
#include <string.h>

// http://www.cse.yorku.ca/~oz/hash.html#djb2
unsigned long uo_strhashtbl_hash(
    const unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uo_strhashtbl *uo_strhashtbl_create(
    size_t capacity)
{
    uo_strhashtbl *strhashtbl = malloc(sizeof *strhashtbl);
    strhashtbl->items = calloc(strhashtbl->capacity = capacity, sizeof *strhashtbl->items);
    return strhashtbl;
}

void uo_strhashtbl_destroy(
    uo_strhashtbl *strhashtbl)
{
    free(strhashtbl->items);
    free(strhashtbl);
}

static struct uo_strkvp *uo_strhashtbl_find_item(
    const uo_strhashtbl *strhashtbl, 
    const void *key)
{
    unsigned long h = uo_strhashtbl_hash((const unsigned char *)key) % strhashtbl->capacity;
    struct uo_strkvp *kvp = strhashtbl->items + h;
    while (kvp->key && strcmp(kvp->key, key))
        kvp = strhashtbl->items + ++h % strhashtbl->capacity;
    return kvp;
}

void uo_strhashtbl_insert(
    uo_strhashtbl *strhashtbl, 
    const char *key, 
    void *value)
{
    struct uo_strkvp *kvp = uo_strhashtbl_find_item(strhashtbl, key);
    kvp->key = key;
    kvp->value = value;
}

void *uo_strhashtbl_remove(
    uo_strhashtbl *strhashtbl, 
    const char *key)
{
    struct uo_strkvp *kvp = uo_strhashtbl_find_item(strhashtbl, key);
    kvp->key = NULL;
    return kvp->value;
}

void *uo_strhashtbl_find(
    uo_strhashtbl *strhashtbl,
    const char *key)
{
    return uo_strhashtbl_find_item(strhashtbl, key)->value;
}

