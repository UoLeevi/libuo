#include "uo_json.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool test_uo_json_decode_utf8(void)
{
    bool passed = true;

    char *json_uf8 = "\""
        "\\u0391\\u0392\\u0393\\u0394\\u0395\\u0396\\u0397\\u0398\\u0399\\u039a\\u039b\\u039c\\u039d\\u039e\\u039f\\u03a0"
        "\\u03a1\\u03a3\\u03a4\\u03a5\\u03a6\\u03a7\\u03a8\\u03a9\\u03b1\\u03b2\\u03b3\\u03b4\\u03b5\\u03b6\\u03b7\\u03b8"
        "\\u03b9\\u03ba\\u03bb\\u03bc\\u03bd\\u03be\\u03bf\\u03c0\\u03c1\\u03c2\\u03c3\\u03c4\\u03c5\\u03c6\\u03c7\\u03c8\\u03c9" "\"";

    char utf8[99];
    char *end;

    end = uo_json_decode_utf8(utf8, json_uf8, strlen(json_uf8));
    *end = '\0';

    passed &= strcmp(utf8, "ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρςστυφχψω") == 0;

    end = uo_json_decode_utf8(utf8, "\"" "ABCDEF" "\"", strlen( "\"" "ABCDEF" "\""));
    *end = '\0';

    passed &= strcmp(utf8, "ABCDEF") == 0;

    return passed;
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= test_uo_json_decode_utf8();

    return passed ? 0 : 1;
}
