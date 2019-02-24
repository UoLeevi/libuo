#include "uo_json.h"

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

    const char *json1 = " {}";
    passed &= uo_json_find_end(json1) - json1 == 3;

    const char *json2 = "[]";
    passed &= uo_json_find_end(json2) - json2 == 2; 

    const char *json3 = "\"\"";
    passed &= uo_json_find_end(json3) - json3 == 2; 

    const char *json4 = "  { ";
    passed &= uo_json_find_end(json4) == NULL;

    const char *json5 =
        " {"
        " \"test\": [],"
        " \"asdf\": -123.123e+123, "
        " \"qwer\": {"
        "     \"zxcv\": null, "
        "     \"asdf\": \"\\\"\""
        "   } "
        " } ";
    passed &= uo_json_find_end(json5) - json5 == 91;
    passed &= uo_json_find_value(json5, "asdf") - json5 == 23;

    return passed ? 0 : 1;
}
