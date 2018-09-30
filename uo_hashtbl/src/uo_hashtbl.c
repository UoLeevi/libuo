#include "uo_hashtbl.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uo_kvp *uo_hashtbl_find_item(
    const uo_hashtbl *hashtbl, 
    const void *key)
{
    uint64_t h = hashtbl->hash(key) % hashtbl->capasity;
    uo_kvp *kvp = hashtbl->items + h;
    while (kvp->key && !hashtbl->equals(kvp->key, key))
        kvp = hashtbl->items + ++h % hashtbl->capasity;
    return kvp;
}

uo_hashtbl *uo_hashtbl_create(
    const size_t capasity, 
    uint64_t (*hash)(const void *), 
    bool (*equals)(const void *, const void *))
{
    uo_hashtbl *hashtbl = malloc(sizeof(uo_hashtbl));
    hashtbl->items = malloc(capasity * sizeof(uo_kvp));
    memset(hashtbl->items, 0, capasity * sizeof(uo_kvp));
    hashtbl->capasity = capasity;
    hashtbl->hash = hash;
    hashtbl->equals = equals;
    return hashtbl;
}

void uo_hashtbl_destroy(
    uo_hashtbl *hashtbl)
{
    free(hashtbl->items);
    free(hashtbl);
}

void uo_hashtbl_insert(
    const uo_hashtbl *hashtbl, 
    void *key, 
    void *value)
{
    uo_kvp *kvp = uo_hashtbl_find_item(hashtbl, key);
    kvp->key = key;
    kvp->value = value;
}

void *uo_hashtbl_remove(
    const uo_hashtbl *hashtbl,
    const void *key)
{
    uo_kvp *kvp = uo_hashtbl_find_item(hashtbl, key);
    kvp->key = NULL;
    return kvp->value;
}

void *uo_hashtbl_find(
    const uo_hashtbl *hashtbl, 
    const void *key)
{
    uo_kvp *kvp = uo_hashtbl_find_item(hashtbl, key);
    return kvp->value;
}
