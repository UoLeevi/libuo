#ifndef UO_BASE64_H
#define UO_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

size_t uo_base64_len(
    size_t len);

char *uo_base64_encode(
    char *dst, 
    const void *src, 
    size_t src_len);

void *uo_base64_decode(
    void *dst, 
    const char *src, 
    size_t src_len);

#ifdef __cplusplus
}
#endif

#endif