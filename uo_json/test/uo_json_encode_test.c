#include "uo_json.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static char json[0x1000];

bool uo_json_encode_number_test(void)
{
    bool passed = true;
    char *end;

    end = uo_json_encode(json, 1);
    *end = '\0';
    passed &= strcmp(json, "1") == 0;

    end = uo_json_encode(json, 1.234);
    *end = '\0';
    passed &= strcmp(json, "1.234") == 0;

    end = uo_json_encode(json, 0.00001);
    *end = '\0';
    passed &= (strcmp(json, "1e-05") == 0 || strcmp(json, "1e-005") == 0);

    end = uo_json_encode(json, -100000000000);
    *end = '\0';
    passed &= strcmp(json, "-100000000000") == 0;

    bool t = true;
    end = uo_json_encode(json, t);
    *end = '\0';
    passed &= strcmp(json, "true") == 0;

    bool f = false;
    end = uo_json_encode(json, f);
    *end = '\0';
    passed &= strcmp(json, "false") == 0;

    return passed;
}

bool uo_json_encode_utf8_test(void)
{
    bool passed = true;
    char *end;

    end = uo_json_encode(json,
        "UTF-8 is a variable width character encoding capable of encoding all 1,112,064 "
        "valid code points in Unicode using one to four 8-bit bytes.");
    *end = '\0';
    passed &= strcmp(json,
        "\"UTF-8 is a variable width character encoding capable of encoding all 1,112,064 "
        "valid code points in Unicode using one to four 8-bit bytes.\"") == 0;

    end = uo_json_encode(json,
        "€");
    *end = '\0';
    passed &= strcmp(json,
        "\"€\"") == 0;

    end = uo_json_encode(json,
        "\n");
    *end = '\0';
    passed &= strcmp(json,
        "\"\\u000A\"") == 0;

    end = uo_json_encode(json,
        NULL);
    *end = '\0';
    passed &= strcmp(json,
        "null") == 0;

    end = uo_json_encode(json,
        ((char[]) { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, '\0' }));
    *end = '\0';
    passed &= strcmp(json,
        "\"\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007\\u0008\\u0009\\u000A\\u000B\\u000C\\u000D\\u000E\\u000F\"") == 0;

    end = uo_json_encode_kvp(json,
        "key", "value");
    *end = '\0';
    passed &= strcmp(json,
        "\"key\": \"value\"") == 0;

    end = uo_json_encode_kvp(json,
        "key", 1);
    *end = '\0';
    passed &= strcmp(json,
        "\"key\": 1") == 0;

    return passed;
}

int main(
    int argc, 
    char const **argv)
{
    char test = argc > 1
        ? argv[1][0]
        : '\0';

    switch (test)
    {
        case 'n': return uo_json_encode_number_test() ? 0 : 1;
        case 'u': return uo_json_encode_utf8_test() ? 0 : 1;
        default: return uo_json_encode_number_test()
            && uo_json_encode_utf8_test() ? 0 : 1;
    }
}
