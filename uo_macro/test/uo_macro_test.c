#include "uo_macro.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;
    
    passed &= uo_narg(a, b, c) == 3;
    passed &= strcmp(uo_nameof(passed), "passed") == 0;

    return passed ? 0 : 1;
}
