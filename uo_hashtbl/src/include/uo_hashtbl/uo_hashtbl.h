#ifndef UO_HASHTBL_H
#define UO_HASHTBL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct uo_kvp {
    void *key;
    void *value;
} uo_kvp;

typedef struct uo_hashtbl {
    size_t capasity;
    uint64_t (*hash)(const void *);
    bool (*equals)(const void *, const void *);
    uo_kvp *items;
} uo_hashtbl;

uo_hashtbl *uo_hashtbl_create(
    const size_t capasity, 
    uint64_t (*hash)(const void *), 
    bool (*equals)(const void *, const void *));

void uo_hashtbl_destroy(
    uo_hashtbl *);

void uo_hashtbl_insert(
    const uo_hashtbl *, 
    void *key, 
    void *value);

void *uo_hashtbl_remove(
    const uo_hashtbl *, 
    const void *key);

void *uo_hashtbl_find(
    const uo_hashtbl *,
    const void *key);

#ifdef __cplusplus
}
#endif

#endif