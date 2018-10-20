#ifndef UO_MEM_H
#define UO_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#define UO_CAT(x, y) x ## y
#define UO_EVALCAT(x, y) UO_CAT(x, y)
#define UO_VAR(ident) UO_EVALCAT(ident, __LINE__)

#define uo_mem_write(dst, src, len) \
{ \
    const size_t UO_VAR(len_) = (len); \
    (dst) = (void *)(((char *)memcpy((dst), (src), UO_VAR(len_))) + UO_VAR(len_)); \
}

#define uo_mem_using(p, size) \
    for (int UO_VAR(once) = 1; UO_VAR(once);) \
        for (void *(p) = malloc(size); UO_VAR(once); free(p), UO_VAR(once) = 0)

#ifdef __cplusplus
}
#endif

#endif