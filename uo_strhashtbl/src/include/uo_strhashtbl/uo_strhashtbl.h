#ifndef UO_STRHASHTBL_H
#define UO_STRHASHTBL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct uo_strhashtbl 
{
    size_t capacity;
    struct uo_strkvp
    {
        const char *key;
        void *value;
    } *items;
} uo_strhashtbl;

uo_strhashtbl *uo_strhashtbl_create(
    size_t capacity);

void uo_strhashtbl_destroy(
    uo_strhashtbl *);

void uo_strhashtbl_insert(
    uo_strhashtbl *, 
    const char *key, 
    void *value);

void *uo_strhashtbl_remove(
    uo_strhashtbl *, 
    const char *key);

void *uo_strhashtbl_find(
    uo_strhashtbl *,
    const char *key);

#ifdef __cplusplus
}
#endif

#endif