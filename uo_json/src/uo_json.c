#include "uo_json.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

#define UO_JSON_BEGIN_ARRAY     '['
#define UO_JSON_BEGIN_OBJECT    '{'
#define UO_JSON_END_ARRAY       ']'
#define UO_JSON_END_OBJECT      '}'
#define UO_JSON_NAME_SEPARATOR  ':'
#define UO_JSON_VALUE_SEPARATOR ','

char *uo_json_encode_utf8(
    char *dst,
    const char *src,
    size_t src_len)
{
    if (!src)
        return uo_json_encode_null(dst);

    const unsigned char *u8 = (unsigned char *)src;
    unsigned char *p = (unsigned char *)dst;
    *p++ = '"';

    while (src_len--)
    {
        unsigned char c = *u8++;
        
        if (iscntrl(c))
        {
            p += sprintf(p, "\\u%04X", c);
            continue;
        }

        if (c == '"' || c == '\\')
            *p++ = '\\';

        *p++ = c;
    }

    *p++ = '"';
    return p;
}

char *uo_json_encode_uint64(
    char *dst,
    uint64_t uint64)
{
    return dst + sprintf(dst, "%" PRIu64, uint64);
}

char *uo_json_encode_int64(
    char *dst,
    int64_t int64)
{
    return dst + sprintf(dst, "%" PRId64, int64);
}

char *uo_json_encode_double(
    char *dst,
    double d)
{
    return dst + sprintf(dst, "%g", d);
}

char *uo_json_encode_bool(
    char *dst,
    bool b)
{
    return dst + sprintf(dst, "%s", b ? "true" : "false");
}

char *uo_json_encode_null(
    char *dst)
{
    return dst + sprintf(dst, "%s", "null");
}
