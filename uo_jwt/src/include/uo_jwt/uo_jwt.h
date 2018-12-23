#ifndef UO_JWT_H
#define UO_JWT_H

#ifdef __cplusplus
extern "C" {
#endif

// https://tools.ietf.org/html/rfc7519

typedef char *uo_jwt;

uo_jwt uo_jwt_hs256_create(
    const char *iss,
    const char *exp,
    const char *sub,
    const char *aud,
    const char *secret);

#ifdef __cplusplus
}
#endif

#endif