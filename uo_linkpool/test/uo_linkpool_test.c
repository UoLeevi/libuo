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

    passed &= !int_linkpool_is_init();

    passed &= int_linkpool_init();

    passed &= int_linkpool_is_init();
    passed &= int_linkpool_is_empty();

    int_link *i0 = int_linkpool_rent();
    int_link *i1 = int_linkpool_rent();
    int_link *i2 = int_linkpool_rent();

    passed &= !int_linkpool_is_empty();

    i0->item = 12345;
    passed &= i0->item == 12345;

    passed &= testfunc_linkpool_init();

    testfunc_link *f0 = testfunc_linkpool_rent();
    testfunc_link *f1 = testfunc_linkpool_rent();
    testfunc_linkpool_return(f0);
    f0 = testfunc_linkpool_rent();

    f0->item = testfunc_increment;

    f0->item(&i0->item);

    passed &= i0->item == 12346;

    int_linkpool_return(i0);
    int_linkpool_return(i1);
    int_linkpool_return(i2);

    passed &= !int_linkpool_is_empty();

    int_linkpool_quit();
    testfunc_linkpool_quit();

    return passed ? 0 : 1;
}
