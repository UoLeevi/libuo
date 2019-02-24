#include "uo_jwt.h"
#include "uo_json.h"
#include "uo_base64.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *uo_jwt_hs256_append_header(
    char *dst)
{
    char *p = dst;
    p = uo_base64url_encode(p, "{ \"alg\": \"HS256\", \"typ\": \"JWT\" }", 32);
    p = uo_mem_append_str_literal(p, ".{ ");
    return p;
}

char *uo_jwt_hs256_append_signature(
    char *dst,
    char *jwt_header,
    const char *key,
    size_t key_len)
{
    char *p = dst - 2;
    p = uo_mem_append_str_literal(p, " }");

    char *jwt_payload = jwt_header + 44;
    p = uo_base64url_encode(jwt_payload, jwt_payload, p - jwt_payload);
    p = uo_mem_append_str_literal(p, ".");

    HMAC(EVP_sha256(), 
        key, key_len,
        jwt_header, (p - 1) - jwt_header,
        p, NULL);

    p = uo_base64url_encode(p, p, 32);
    *p = '\0';

    return p;
}

bool uo_jwt_verify(
    const char *jwt,
    size_t jwt_len,
    const char *key,
    size_t key_len)
{
    char *header_end = memchr(jwt, '.', jwt_len);
    if (!header_end)
        return false;

    char *payload = header_end + 1;
    char *payload_end = memchr(payload, '.', jwt_len - (jwt - payload));
    if (!payload_end)
        return false;

    char *signature = payload_end + 1;
    if (jwt + jwt_len - signature != 43)
        return false;

    char hs256[43]; 
    // hmac sha256 output length is 32 bytes.
    // base64url encoding increases the size by 1/3 to 43 bytes

    HMAC(EVP_sha256(), 
        key, key_len,
        jwt, payload_end - jwt,
        hs256, NULL);

    uo_base64url_encode(hs256, hs256, 32);
    
    return memcmp(signature, hs256, 43) == 0;
}

char *uo_jwt_decode_payload(
    char *dst,
    char *jwt,
    size_t jwt_len)
{
    char *jwt_payload = memchr(jwt, '.', jwt_len - 1);

    if (!jwt_payload)
        return NULL;

    ++jwt_payload;

    char *jwt_payload_end = memchr(jwt_payload, '.', jwt_len - (jwt_payload - jwt));

    if (!jwt_payload_end)
        return NULL;

    if (!dst)
        dst = jwt_payload;

    jwt_payload_end = uo_base64url_decode(dst, jwt_payload, jwt_payload_end - jwt_payload);

    if (!jwt_payload_end)
        return NULL;

    *jwt_payload_end = '\0';

    return dst;
}
