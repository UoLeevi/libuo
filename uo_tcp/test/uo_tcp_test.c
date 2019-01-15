#include "uo_tcp.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

int main(
    int argc, 
    char **argv)
{
    bool pass = true;

    pass &= uo_tcp_init();

    return pass ? 0 : 1;
}

