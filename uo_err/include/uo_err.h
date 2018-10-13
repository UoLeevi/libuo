#ifndef UO_ERR_H
#define UO_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#define UO_CAT(x,y) x ## y
#define UO_EVALCAT(x,y) UO_CAT(x,y)
#define UO_VAR(ident) UO_EVALCAT(ident, __LINE__)

#define UO_ONERR(expr, val, onerr_expr) \
    for (int UO_VAR(once) = 1; UO_VAR(once);) \
        for (; UO_VAR(once); onerr_expr, UO_VAR(once) = 0)

#undef UO_CAT
#undef UO_EVALCAT
#undef UO_VAR

void uo_err(
    const char *fmt,
    ...);

void uo_err_exit(
    const char *fmt,
    ...);

#define uo_err_return( \
    return_val, \
    fmt, \
    ...) \
{ \
    uo_err(fmt, ##__VA_ARGS__); \
    return (return_val); \
}

#ifdef __cplusplus
}
#endif

#endif