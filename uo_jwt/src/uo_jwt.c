#include "uo_jwt.h"
#include "uo_json.h"
#include "uo_base64.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uo_jwt uo_jwt_hs256_create(
    const char *iss,
    const char *exp,
    const char *sub,
    const char *aud,
    const char *secret)
{
    char *jwt = malloc(0x1000);
    char *p, *header, *payload, *signature;

    header = p = jwt;

    p = uo_base64url_encode(header, 
        "{ \"alg\": \"HS256\", \"typ\": \"JWT\" }", 32);

    *p++ = '.';
    payload = p;

    memcpy(p, "{ \"iss\": ", 8);
    p += 8;
    p = uo_json_encode(p, iss);

    memcpy(p, ", \"exp\": ", 8);
    p += 8;
    p = uo_json_encode(p, exp);

    memcpy(p, ", \"sub\": ", 8);
    p += 8;
    p = uo_json_encode(p, sub);
    
    memcpy(p, ", \"aud\": ", 8);
    p += 8;
    p = uo_json_encode(p, aud);
    *p++ = ' ';
    *p++ = '}';

    p = uo_base64url_encode(payload, payload, p - payload);
    
    *p++ = '.';
    signature = p;

    unsigned int signature_len;
    HMAC(EVP_sha256(), 
        secret, strlen(secret),
        jwt, (p - 1) - jwt,
        signature, &signature_len);

    p = uo_base64url_encode(signature, signature, signature_len);
    *p = '\0';

    return jwt;
}
