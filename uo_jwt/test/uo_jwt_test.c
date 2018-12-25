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

    char jwt[0x1000];

    char *p = uo_jwt_hs256_append_header(jwt);
    p = uo_jwt_append_claim(p, "iss", "asdf");
    p = uo_jwt_append_claim(p, "exp", 1300819380);
    p = uo_jwt_append_claim(p, "sub", "asdfgh");
    p = uo_jwt_append_claim(p, "aud", "asdfghj");
    p = uo_jwt_hs256_append_signature(p, jwt, "asdfghjk", 8);

    passed &= strcmp(jwt, 
        "eyAiYWxnIjogIkhTMjU2IiwgInR5cCI6ICJKV1QiIH0."
        "eyAiaXNzIjogImFzZGYiLCAiZXhwIjogMTMwMDgxOTM4MCwgInN1YiI6ICJhc2RmZ2giLCAiYXVkIjogImFzZGZnaGoiIH0."
        "cE6hcQlQh7gjNWHj3J41O6Cx0PXykeMZCHkYKq5JYgg") == 0;

    printf("%s", jwt);
    passed &= uo_jwt_verify(jwt, 183, "asdfghjk", 8);

    return passed ? 0 : 1;
}
