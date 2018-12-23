#include "uo_jwt.h"

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

    char *jwt = uo_jwt_hs256_create(
        "asdf",
        "asdfg",
        "asdfgh",
        "asdfghj",
        "asdfghjk");

    passed &= strcmp(jwt, 
        "eyAiYWxnIjogIkhTMjU2IiwgInR5cCI6ICJKV1QiIH0."
        "eyAiaXNzIjoiYXNkZiIsICJleHAiOiJhc2RmZyIsICJzdWIiOiJhc2RmZ2giLCAiYXVkIjoiYXNkZmdoaiIgfQ."
        "C6vFMTZfKWcREaOfcFgzp2jaarwKiwLrJE3dfAFgJd4") == 0;

    return passed ? 0 : 1;
}
