#ifndef UO_MEM_H
#define UO_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_macro.h"

#include <stdlib.h>
#include <string.h>

static _Thread_local size_t temp_size;
static _Thread_local const void *temp_ptr;

#define uo_mem_write(dst, src, len) \
{ \
    const size_t UO_VAR(len_) = (len); \
    (dst) = (void *)(((char *)memcpy((dst), (src), UO_VAR(len_))) + UO_VAR(len_)); \
}

#define uo_mem_append(dst, src, len) \
( \
    temp_ptr = (src), \
    temp_size = (len), \
    _Generic((dst), \
                 char *: (char *)memcpy((dst), temp_ptr, temp_size) + temp_size, \
        unsigned char *: (unsigned char *)memcpy((dst), temp_ptr, temp_size) + temp_size, \
                default: (void *)((char *)memcpy((dst), temp_ptr, temp_size) + temp_size)) \
)

#define uo_mem_append_str_literal(dst, str_literal) \
    uo_mem_append(dst, str_literal, UO_STRLEN(str_literal))

#define uo_mem_append_str(dst, str) \
    uo_mem_append(dst, str_literal, strlen(str_literal))

#define uo_mem_cmp_str_literal(str, str_literal) \
    memcmp((str), (str_literal), UO_STRLEN(str_literal))

#define uo_mem_using(p, size) \
    for (int UO_VAR(once) = 1; UO_VAR(once);) \
        for (void *(p) = malloc(size); UO_VAR(once); free(p), UO_VAR(once) = 0)

#ifdef __cplusplus
}
#endif

#endif
