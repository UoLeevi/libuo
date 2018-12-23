#ifndef UO_JSON_H
#define UO_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

// https://tools.ietf.org/html/rfc8259

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

char *uo_json_encode_utf8(
    char *dst,
    const char *src,
    size_t src_len);

char *uo_json_encode_uint64(
    char *dst,
    uint64_t uint64);

char *uo_json_encode_int64(
    char *dst,
    int64_t int64);

char *uo_json_encode_double(
    char *dst,
    double d);

char *uo_json_encode_bool(
    char *dst,
    bool b);

char *uo_json_encode_null(
    char *dst);

char *uo_json_encode_noop(
    void *,
    ...);

#define uo_json_encode_utf8_cstring(dst, utf8) \
    uo_json_encode_utf8(dst, (const char *)(utf8), (utf8) ? strlen((const char *)(utf8)) : 0)

#define uo_json_encode_number(dst, num) _Generic((num), \
                      char: uo_json_encode_int64, \
                 short int: uo_json_encode_int64, \
                       int: uo_json_encode_int64, \
                  long int: uo_json_encode_int64, \
             long long int: uo_json_encode_int64, \
             unsigned char: uo_json_encode_uint64, \
        unsigned short int: uo_json_encode_uint64, \
              unsigned int: uo_json_encode_uint64, \
         unsigned long int: uo_json_encode_uint64, \
    unsigned long long int: uo_json_encode_uint64, \
                     float: uo_json_encode_double, \
                    double: uo_json_encode_double, \
               long double: uo_json_encode_double, \
                   default: uo_json_encode_noop \
)(dst, num)

#define uo_json_encode(dst, val) _Generic((val), \
                     _Bool: uo_json_encode_bool(dst, val), \
                    char *: uo_json_encode_utf8_cstring(dst, (uintptr_t)val), \
                    const char *: uo_json_encode_utf8_cstring(dst, (uintptr_t)val), \
                    void *: uo_json_encode_utf8_cstring(dst, (uintptr_t)val), \
                   default: uo_json_encode_number(dst, val) \
)

#ifdef __cplusplus
}
#endif

#endif