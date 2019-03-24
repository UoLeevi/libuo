#include "uo_refstack.h"

#include <stdbool.h>
#include <stddef.h>

static int counter;

void test_finalizer(
    void *arg)
{
    ++counter;
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    uo_refcount_linkpool_init();
    uo_refcount_linkpool_thrd_init();

    uo_refstack refstack;
    uo_refstack_create_at(&refstack);

    uo_refstack_push(&refstack, NULL, test_finalizer);
    passed &= counter == 0;

    uo_refstack_finalize(&refstack);
    passed &= counter == 1;

    uo_refstack_push(&refstack, &refstack, (void (*)(void *))uo_refstack_destroy_at);
    uo_refstack_push(&refstack, NULL, test_finalizer);
    passed &= counter == 1;

    uo_refstack_finalize(&refstack);
    passed &= counter == 2;

    uo_refcount_linkpool_thrd_quit();

    return passed ? 0 : 1;
}
