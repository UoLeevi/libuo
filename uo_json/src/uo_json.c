#include "uo_json.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

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

static bool uo_json_is_char_escaped(
    const char *json_char)
{
    bool is_escaped = false;

    while (*--json_char == '\\')
        is_escaped = !is_escaped;

    return is_escaped;
}

static char *uo_json_find_num_end(
    const char *json_num)
{
    const char *p = json_num;

    if (*p == '-')
        ++p;

    while (isdigit(*p))
        ++p;

    if (!*p)
        return (char *)p;
    
    if (*p == '.')
        ++p;
    
    while (isdigit(*p))
        ++p;

    if (tolower(*p) == 'e')
    {
        ++p;
        if (*p != '+' && *p != '-')
            return NULL;
        ++p;

        while (isdigit(*p))
            ++p;
    }

    switch (*p)
    {
        case '\0':
        case ',':
            return (char *)p;
    
        default: 
            return isspace(*p) ? (char *)p : NULL;
    }
}

static char *uo_json_find_str_end(
    const char *json_str)
{
    const char *quote = strchr(json_str + 1, '\"');

    while (quote && uo_json_is_char_escaped(quote))
        quote = strchr(quote + 1, '\"');

    return quote
        ? (char *)quote + 1
        : NULL;
}

static char *uo_json_find_obj_end(
    const char *json_obj)
{
    const char *p = json_obj + 1;

    while (*p)
    {
        const char *right_brace = strchr(p, '}');
        if (!right_brace)
            return NULL;

        const char *key = strchr(p, '\"');
        if (!key || right_brace < key)
            return (char *)right_brace + 1;

        const char *key_end = uo_json_find_str_end(key);
        if (!key_end)
            return NULL;

        p = strchr(key_end, ':');
        if (!p)
            return NULL;

        ++p;

        while (isspace(*p))
            ++p;

        p = uo_json_find_end(p);
        if (!p)
            return NULL;

        right_brace = strchr(p + 1, '}');
        if (!right_brace)
            return NULL;

        p = strchr(p, ',');
        if (!p)
            return (char *)right_brace + 1;
    }

    return NULL;
}

static char *uo_json_find_arr_end(
    const char *json_arr)
{
    const char *p = json_arr + 1;

    const char *right_bracket = strchr(p, ']');
    if (!right_bracket)
        return NULL;

    if (p == right_bracket)
        return (char *)right_bracket + 1;

    while (*p)
    {
        while (isspace(*p))
            ++p;

        p = uo_json_find_end(p);
        if (!p)
            return NULL;

        right_bracket = strchr(p + 1, ']');
        if (!right_bracket)
            return NULL;

        p = strchr(p, ',');
        if (!p)
            return (char *)right_bracket + 1;
    }

    return NULL;
}

char *uo_json_find_end(
    const char *json)
{
    while (isspace(*json))
        ++json;

    switch (*json)
    {
        case '\0': return NULL;
        case '{':  return uo_json_find_obj_end(json);
        case '[':  return uo_json_find_arr_end(json);
        case '"':  return uo_json_find_str_end(json);
        case 'n':  return (char *)json + UO_STRLEN("null");
        case 't':  return (char *)json + UO_STRLEN("true");
        case 'f':  return (char *)json + UO_STRLEN("false");
        default:   return uo_json_find_num_end(json);
    }
}

char *uo_json_find_value(
    const char *json,
    const char *name)
{
    size_t name_len = strlen(name);

    const char *p = json + 1;

    while (isspace(*p))
        ++p;

    if (*p != '{')
        return NULL;

    while (*p)
    {
        const char *right_brace = strchr(p, '}');
        if (!right_brace)
            return NULL;

        const char *key = strchr(p, '\"');
        if (!key || right_brace < key)
            return NULL;

        const char *key_end = uo_json_find_str_end(key);
        if (!key_end)
            return NULL;

        bool is_match = name_len + 2 == key_end - key
            && memcmp(name, key + 1, name_len) == 0;

        p = strchr(key_end, ':');
        if (!p)
            return NULL;

        ++p;

        while (isspace(*p))
            ++p;

        if (is_match)
            return (char *)p;

        p = uo_json_find_end(p);
        if (!p)
            return NULL;

        right_brace = strchr(p + 1, '}');
        if (!right_brace)
            return NULL;

        p = strchr(p, ',');
        if (!p)
            return NULL;
    }

    return NULL;
}
