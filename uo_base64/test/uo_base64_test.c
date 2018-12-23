#include "uo_base64.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool test1(void)
{
    const char *binary =
        "Man is distinguished, not only by his reason, but by this singular passion from "
        "other animals, which is a lust of the mind, that by a perseverance of delight "
        "in the continued and indefatigable generation of knowledge, exceeds the short "
        "vehemence of any carnal pleasure.";

    const char *expected_base64 =
        "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
        "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
        "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
        "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
        "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

    bool passed = true;

    size_t binary_len = strlen(binary);
    size_t base64_len = uo_base64_len(binary_len);

    // length test
    passed &= strlen(expected_base64) == base64_len;

    // encoding test
    char *actual_base64 = malloc(base64_len + 1);
    char *a = uo_base64_encode(actual_base64, binary, binary_len);
    *a = '\0';
    passed &= strcmp(expected_base64, actual_base64) == 0;

    // decoding tests
    char *actual_binary = actual_base64;
    char *b = uo_base64_decode(actual_binary, actual_base64, base64_len);
    *b = '\0';
    passed &= strcmp(binary, actual_binary) == 0;

    return passed;
}

bool test2(void)
{
    const char jwt_header_json[] = {
        123, 34, 116, 121, 112, 34, 58, 34, 74, 87, 84, 34, 44, 13, 10, 32,
        34, 97, 108, 103, 34, 58, 34, 72, 83, 50, 53, 54, 34, 125, 0};
    const char jwt_payload_json[] = {
        123, 34, 105, 115, 115, 34, 58, 34, 106, 111, 101, 34, 44, 13, 10,
        32, 34, 101, 120, 112, 34, 58, 49, 51, 48, 48, 56, 49, 57, 51, 56,
        48, 44, 13, 10, 32, 34, 104, 116, 116, 112, 58, 47, 47, 101, 120, 97,
        109, 112, 108, 101, 46, 99, 111, 109, 47, 105, 115, 95, 114, 111,
        111, 116, 34, 58, 116, 114, 117, 101, 125, 0};

    const char *jwt_expected =
        "eyJ0eXAiOiJKV1QiLA0KICJhbGciOiJIUzI1NiJ9."
        "eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ";

    bool passed = true;

    size_t binary_len = sizeof jwt_header_json + sizeof jwt_payload_json - 2;
    size_t base64_len = uo_base64_len(binary_len);

    // encoding test
    char *jwt_actual = malloc(base64_len + 2);
    char *a = jwt_actual;
    a = uo_base64url_encode(a, jwt_header_json, strlen(jwt_header_json));
    *a++ = '.';
    a = uo_base64url_encode(a, jwt_payload_json, strlen(jwt_payload_json));
    *a = '\0';
    passed &= strcmp(jwt_expected, jwt_actual) == 0;

    printf("actual: \n%s\nexpected:\n%s\n", jwt_actual, jwt_expected);

    return passed;
}

int main(
    int argc,
    char const **argv)
{
    bool passed = true;

    passed &= uo_base64_len(0) == 0;
    passed &= uo_base64_len(1) == 4;
    passed &= uo_base64_len(2) == 4;
    passed &= uo_base64_len(3) == 4;
    passed &= uo_base64_len(4) == 8;
    passed &= uo_base64_len(5) == 8;
    passed &= uo_base64_len(6) == 8;

    passed &= test1();
    passed &= test2();

    return passed ? 0 : 1;
}
