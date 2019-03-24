#include "uo_refcount.h"

uo_impl_linkpool(uo_refcount, uo_refcount);

uo_refcount *uo_refcount_create(
    void *ptr,
    void (*finalizer)(void *))
{
    uo_refcount *refcount = &uo_refcount_linkpool_rent()->item;
    atomic_init(&refcount->count, 1);
    refcount->ptr = ptr;
    refcount->finalizer = finalizer;
    return refcount;
}

void uo_refcount_dec(
    uo_refcount *refcount)
{
    if (atomic_fetch_sub(&refcount->count, 1) == 1)
    {
        refcount->finalizer(refcount->ptr);
        uo_refcount_linkpool_return(uo_refcount_get_linklist(refcount));
    }
}
