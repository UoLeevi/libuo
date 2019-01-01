#ifndef UO_BUF_H
#define UO_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>

typedef unsigned char *uo_buf;

uo_buf uo_buf_alloc(
    size_t size);

uo_buf uo_buf_realloc(
    uo_buf,
    size_t size);

uo_buf uo_buf_realloc_2x(
    uo_buf);

void uo_buf_free(
    uo_buf);

void uo_buf_null_terminate(
    uo_buf *);

void *uo_buf_memcpy_append(
    uo_buf *restrict buf,
    const void *restrict src, 
    size_t size);

int uo_buf_printf_append(
    uo_buf *,
    const char *format,
    ...);

unsigned char *uo_buf_get_ptr(
    uo_buf);

void uo_buf_set_ptr_rel(
    uo_buf,
    ssize_t);

void uo_buf_set_ptr_abs(
    uo_buf,
    size_t);

size_t uo_buf_get_len_before_ptr(
    uo_buf);

size_t uo_buf_get_len_after_ptr(
    uo_buf);

size_t uo_buf_get_size(
    uo_buf);

unsigned char *uo_buf_get_end(
    uo_buf);

#ifdef __cplusplus
}
#endif

#endif