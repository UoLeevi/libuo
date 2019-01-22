#include "uo_io.h"

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

    passed &= uo_io_init();

    return passed ? 0 : 1;
}
