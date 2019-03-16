#include "uo_linkpool.h"

#include <stdbool.h>
#include <stddef.h>

typedef void (*testfunc)(int *);

uo_def_linkpool(int);
uo_def_linkpool(testfunc);

void testfunc_increment(int *i)
{
    ++*i;
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= int_linkpool_init();
    passed &= !int_linkpool_is_thrd_init();
    passed &= int_linkpool_thrd_init();
    passed &= int_linkpool_is_thrd_init();
    passed &= int_linkpool_is_empty();

    int_linklist *i0 = int_linkpool_rent();
    int_linklist *i1 = int_linkpool_rent();
    int_linklist *i2 = int_linkpool_rent();

    passed &= !int_linkpool_is_empty();

    i0->item = 12345;
    passed &= i0->item == 12345;

    passed &= testfunc_linkpool_init();
    passed &= testfunc_linkpool_thrd_init();

    testfunc_linklist *f0 = testfunc_linkpool_rent();
    testfunc_linklist *f1 = testfunc_linkpool_rent();
    testfunc_linkpool_return(f0);
    f0 = testfunc_linkpool_rent();

    f0->item = testfunc_increment;

    f0->item(&i0->item);

    passed &= i0->item == 12346;

    int_linkpool_return(i0);
    int_linkpool_return(i1);
    int_linkpool_return(i2);

    passed &= !int_linkpool_is_empty();

    int_linkpool_thrd_quit();
    testfunc_linkpool_thrd_quit();

    return passed ? 0 : 1;
}
