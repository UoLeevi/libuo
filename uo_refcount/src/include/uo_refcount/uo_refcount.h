#ifndef UO_REFCOUNT_H
#define UO_REFCOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_linklist.h"
#include "uo_linkpool.h"

#include <stdint.h>
#include <stdatomic.h>
#include <assert.h>

typedef struct uo_refcount 
{
    void *ptr;
    void (*finalizer)(void *);
    atomic_uint_fast32_t count;
} uo_refcount;

uo_decl_linklist(uo_refcount, uo_refcount);
uo_decl_linkpool(uo_refcount, uo_refcount);

/**
 * @brief create a reference counted structure
 * 
 */
uo_refcount *uo_refcount_create(
    void *ptr,
    void (*finalizer)(void *));

/**
 * @brief increment the refcount
 * 
 */
static inline void uo_refcount_inc(
    uo_refcount *refcount)
{
    uint32_t count = atomic_fetch_add(&refcount->count, 1);
    assert(count);
}

/**
 * @brief decrement the refcount
 * 
 */
void uo_refcount_dec(
    uo_refcount *);

#ifdef __cplusplus
}
#endif

#endif