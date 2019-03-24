#include "uo_refcount.h"

#include <stdbool.h>
#include <stddef.h>

void test_finalizer(
    void *arg)
{
    ++*(int *)arg;
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    int counter = 0;

    uo_refcount_linkpool_init();
    uo_refcount_linkpool_thrd_init();

    uo_refcount *refcount = uo_refcount_create(&counter, test_finalizer);
    passed &= atomic_load(&refcount->count) == 1;
    passed &= counter == 0;
    uo_refcount_inc(refcount);
    passed &= atomic_load(&refcount->count) == 2;
    passed &= counter == 0;
    uo_refcount_inc(refcount);
    passed &= atomic_load(&refcount->count) == 3;
    passed &= counter == 0;
    uo_refcount_dec(refcount);
    passed &= atomic_load(&refcount->count) == 2;
    passed &= counter == 0;
    uo_refcount_dec(refcount);
    passed &= atomic_load(&refcount->count) == 1;
    passed &= counter == 0;
    uo_refcount_dec(refcount);
    passed &= atomic_load(&refcount->count) == 0;
    passed &= counter == 1;

    uo_refcount_linkpool_thrd_quit();

    return passed ? 0 : 1;
}
