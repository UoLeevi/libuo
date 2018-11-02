#ifndef UO_ERR_H
#define UO_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

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

#define uo_err_goto( \
    goto_label, \
    fmt, \
    ...) \
{ \
    uo_err(fmt, ##__VA_ARGS__); \
    goto goto_label; \
}

#ifdef __cplusplus
}
#endif

#endif