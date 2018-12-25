#ifndef UO_JWT_H
#define UO_JWT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_json.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// https://tools.ietf.org/html/rfc7519

char *uo_jwt_hs256_append_header(
    char *dst);

char *uo_jwt_hs256_append_signature(
    char *dst,
    char *jwt_header,
    const char *key,
    size_t key_len);

#define uo_jwt_append_claim(dst, key, val) \
    uo_mem_append_str_literal( \
        uo_json_encode_kvp(dst, key, val), ", ")

bool uo_jwt_verify(
    const char *jwt,
    size_t jwt_len,
    const char *key,
    size_t key_len);

#ifdef __cplusplus
}
#endif

#endif