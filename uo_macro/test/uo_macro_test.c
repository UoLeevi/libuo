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
    
    passed &= UO_NARG(a, b, c) == 3;

    return passed ? 0 : 1;
}
