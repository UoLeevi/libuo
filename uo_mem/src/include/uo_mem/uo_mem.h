#ifndef UO_MEM_H
#define UO_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_macro.h"

#include <stdlib.h>
#include <string.h>

static _Thread_local size_t temp_size;

#define uo_mem_write(dst, src, len) \
{ \
    const size_t UO_VAR(len_) = (len); \
    (dst) = (void *)(((char *)memcpy((dst), (src), UO_VAR(len_))) + UO_VAR(len_)); \
}

#define uo_mem_append_str_literal(dst, str_literal) _Generic((dst), \
              char *: ((char *)memcpy((dst), (str_literal), UO_STRLEN(str_literal)) + UO_STRLEN(str_literal)), \
     unsigned char *: ((unsigned char *)memcpy((dst), (str_literal), UO_STRLEN(str_literal)) + UO_STRLEN(str_literal)), \
             default: ((void *)((char *)memcpy((dst), (str_literal), UO_STRLEN(str_literal)) + UO_STRLEN(str_literal))))

#define uo_mem_append_str(dst, str) _Generic((dst), \
              char *: ((char *)memcpy((dst), (str), temp_size = strlen(str)) + temp_size), \
     unsigned char *: ((unsigned char *)memcpy((dst), (str), temp_size = strlen(str)) + temp_size), \
             default: ((void *)((char *)memcpy((dst), (str), temp_size = strlen(str)) + temp_size)))

#define uo_mem_using(p, size) \
    for (int UO_VAR(once) = 1; UO_VAR(once);) \
        for (void *(p) = malloc(size); UO_VAR(once); free(p), UO_VAR(once) = 0)

#ifdef __cplusplus
}
#endif

#endif