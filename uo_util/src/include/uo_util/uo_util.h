#ifndef UO_UTIL_H
#define UO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define UO_TEMPSTR_LEN 0x1000

static _Thread_local char uo__tempstr[UO_TEMPSTR_LEN];

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline uint64_t uo_ceil_pow2(
    uint64_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    return n;
}

// http://www.cse.yorku.ca/~oz/hash.html#djb2
static uint64_t uo_strhash_djb2(
    const unsigned char *str)
{
    uint64_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static inline bool uo_streq(
    const char *str1,
    const char *str2)
{
    return strcmp(str1, str2) == 0;
}

#define uo_eq(lhs, rhs) \
    ((lhs) == (rhs))

static inline char *uo_temp_substr(
    const char *str,
    size_t len)
{
    assert(len < UO_TEMPSTR_LEN);
    uo__tempstr[len] = '\0';
    return memcpy(uo__tempstr, str, len);
}

static inline char *uo_temp_strcat(
    const char *str1,
    const char *str2)
{
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    size_t len = str1_len + str2_len;

    assert(len < UO_TEMPSTR_LEN);
    uo__tempstr[len] = '\0';
    memcpy(uo__tempstr + str1_len, str2, str2_len);

    return str1 == uo__tempstr
        ? (char *)str1
        : memcpy(uo__tempstr, str1, str1_len);
}

static char *uo_strchrnth(
    const char *str,
    int ch,
    size_t nth)
{
    assert(nth);
    --str;

    while (str && nth--)
        str = strchr(++str, ch);

    return (char *)str;
}

// https://stackoverflow.com/a/25644105
// https://en.wikipedia.org/wiki/UTF-8#Description
static char *uo_utf8_append(
    char *dst, 
    uint32_t codepoint)
{
    if (codepoint < 0x80)
    {
        *dst++ = (char)(codepoint & 0x7f);
        return dst;
    }

    if (codepoint < 0x800)
    {
        *dst++ = (char)(0xc0 | (codepoint >> 6));
        *dst++ = (char)(0x80 | (codepoint & 0x3f));
        return dst;
    }

    if (codepoint < 0x10000)
    {
        *dst++ = (char)(0xe0 | (codepoint >> 12));
        *dst++ = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        *dst++ = (char)(0x80 | (codepoint & 0x3f));
        return dst;
    }

    if (codepoint < 0x110000) 
    {
        *dst++ = (char)(0xf0 | (codepoint >> 18));
        *dst++ = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        *dst++ = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        *dst++ = (char)(0x80 | (codepoint & 0x3f));
        return dst;
    }

    return NULL;
}

static size_t uo_chrfreq(
    char *str, 
    char chr, 
    char esc_chr)
{
    assert(chr);

    size_t i = 0;
    bool is_prev_esc = false;
    char curr_chr;

    while (curr_chr = *str++)
    {
        if (curr_chr == chr)
            i += !is_prev_esc ? 1 : chr == esc_chr ? -1 : 0;

        is_prev_esc = !is_prev_esc && curr_chr == esc_chr;
    }

    return i;
}

/**
 * @brief get pointer to first character in str1 which is different to str2
 * 
 * If str1 and str2 are identical then NULL is returned.
 */
static inline char *uo_strdiff(
    const char *str1,
    const char *str2)
{
    char chr;

    while ((chr = *str1++) == *str2++)
        if (!chr)
            return NULL;

    return (char *)str1 - 1;
}

static inline bool uo_isprefix(
    const char *pre,
    const char *str)
{
    const char *diff = uo_strdiff(pre, str);

    return !diff || !*diff;
}

static char *uo_uri_decode(
    char *dst,
    const char *src,
    size_t src_len)
{
    // decode URI
    // https://stackoverflow.com/a/14530993

    const char *src_end = src + src_len;

    char a, b;
    while (src != src_end)
    {
        if (*src == '+') 
        {
            *dst++ = ' ';
            src++;
        }
        else if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a') 
                a -= 'a' - 'A';

            if (a >= 'A')
                a -= ('A' - 0xA);
            else 
                a -= '0';

            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 0xA);
            else 
                b -= '0';

            *dst++ = 0x10 * a + b;
            src += 3;
        }
        else
            *dst++ = *src++;
    }

    return dst;
}

#ifdef __cplusplus
}
#endif

#endif
