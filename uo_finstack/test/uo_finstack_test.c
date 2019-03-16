#include "uo_finstack.h"

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

    uo_finstack *finstack = uo_finstack_create();

    uo_finstack_add(finstack, NULL, test_finalizer);
    passed &= counter == 0;

    uo_finstack_finalize(finstack);
    passed &= counter == 1;

    uo_finstack_add(finstack, finstack, (void (*)(void *))uo_finstack_destroy);
    uo_finstack_add(finstack, NULL, test_finalizer);
    passed &= counter == 1;

    uo_finstack_finalize(finstack);
    passed &= counter == 2;

    return passed ? 0 : 1;
}
