#include "uo_base64.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static char *binary =
    "Man is distinguished, not only by his reason, but by this singular passion from "
    "other animals, which is a lust of the mind, that by a perseverance of delight "
    "in the continued and indefatigable generation of knowledge, exceeds the short "
    "vehemence of any carnal pleasure.";

static char *expected_base64 = 
    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
    "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
    "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
    "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
    "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    // length tests
    passed &= uo_base64_len(0) == 0;
    passed &= uo_base64_len(1) == 4;
    passed &= uo_base64_len(2) == 4;
    passed &= uo_base64_len(3) == 4;
    passed &= uo_base64_len(4) == 8;
    passed &= uo_base64_len(5) == 8;
    passed &= uo_base64_len(6) == 8;

    size_t binary_len = strlen(binary);
    size_t base64_len = uo_base64_len(binary_len);
    
    passed &= strlen(expected_base64) == base64_len;


    // encoding tests
    char *actual_base64 = malloc(base64_len + 1);
    char *a = uo_base64_encode(actual_base64, binary, binary_len);
    *a = '\0';

    passed &= strcmp(expected_base64, actual_base64) == 0;


    // decoding tests
    char *actual_binary = actual_base64;
    char *b = uo_base64_decode(actual_binary, actual_base64, base64_len);
    *b = '\0';

    passed &= strcmp(binary, actual_binary) == 0;


    return passed ? 0 : 1;
}
