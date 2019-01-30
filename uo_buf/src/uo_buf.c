#include "uo_buf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uo_buf uo_buf_alloc(
    size_t size)
{
    unsigned char *mem = malloc(sizeof (size_t) * 2 + size);
    ((size_t *)mem)[0] = size;
    ((size_t *)mem)[1] = 0;
    return mem + sizeof (size_t) * 2;
}

uo_buf uo_buf_realloc(
    uo_buf buf,
    size_t size)
{
    unsigned char *mem = realloc((size_t *)buf - 2, sizeof (size_t) * 2 + size);
    ((size_t *)mem)[0] = size;
    return mem + sizeof (size_t) * 2;
}

uo_buf uo_buf_realloc_2x(
    uo_buf buf)
{
    return uo_buf_realloc(buf, uo_buf_get_size(buf) * 2);
}

void uo_buf_free(
    uo_buf buf)
{
    free((size_t *)buf - 2);
}

void uo_buf_null_terminate(
    uo_buf *buf)
{
    if (uo_buf_get_len_after_ptr(*buf) < 1)
        *buf = uo_buf_realloc_2x(*buf);

    *uo_buf_get_ptr(*buf) = '\0';
}

int uo_buf_printf_append(
    uo_buf *buf,
    const char *format,
    ...)
{
    va_list args[2];
    va_start(args[0], format);
    va_copy(args[1], args[0]);

    int size = vsnprintf(NULL, 0, format, args[0]);
    va_end(args[0]);

    while (uo_buf_get_len_after_ptr(*buf) <= size)
        *buf = uo_buf_realloc_2x(*buf);

    vsnprintf(uo_buf_get_ptr(*buf), uo_buf_get_len_after_ptr(*buf), format, args[1]);
    va_end(args[1]);

    uo_buf_set_ptr_rel(*buf, size);

    return size;
}

void *uo_buf_memcpy_append(
    uo_buf *restrict buf,
    const void *restrict src, 
    size_t size)
{
    while (uo_buf_get_len_after_ptr(*buf) <= (ptrdiff_t)size)
        *buf = uo_buf_realloc_2x(*buf);

    void *dst = memcpy(uo_buf_get_ptr(*buf), src, size);

    uo_buf_set_ptr_rel(*buf, size);

    return dst;
}

unsigned char *uo_buf_get_ptr(
    uo_buf buf)
{
    return buf + ((ptrdiff_t *)buf)[-1];
}

void uo_buf_set_ptr_rel(
    uo_buf buf,
    ptrdiff_t rel_offset)
{
    ((ptrdiff_t *)buf)[-1] += rel_offset;
}

void uo_buf_set_ptr_abs(
    uo_buf buf,
    ptrdiff_t abs_offset)
{
    ((ptrdiff_t *)buf)[-1] = abs_offset;
}

ptrdiff_t uo_buf_get_len_before_ptr(
    uo_buf buf)
{
    return ((ptrdiff_t *)buf)[-1];
}

ptrdiff_t uo_buf_get_len_after_ptr(
    uo_buf buf)
{
    return ((ptrdiff_t *)buf)[-2] - ((ptrdiff_t *)buf)[-1];
}

size_t uo_buf_get_size(
    uo_buf buf)
{
    return ((size_t *)buf)[-2];
}

unsigned char *uo_buf_get_end(
    uo_buf buf)
{
    return buf + uo_buf_get_size(buf);
}
