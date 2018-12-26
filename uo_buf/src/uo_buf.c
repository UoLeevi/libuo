#include "uo_buf.h"

#include <stdlib.h>

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

unsigned char *uo_buf_get_ptr(
    uo_buf buf)
{
    return buf + ((size_t *)buf)[-1];
}

void uo_buf_set_ptr_rel(
    uo_buf buf,
    ssize_t rel_offset)
{
    ((size_t *)buf)[-1] += rel_offset;
}

void uo_buf_set_ptr_abs(
    uo_buf buf,
    size_t abs_offset)
{
    ((size_t *)buf)[-1] = abs_offset;
}

size_t uo_buf_get_len_before_ptr(
    uo_buf buf)
{
    return ((size_t *)buf)[-1];
}

size_t uo_buf_get_len_after_ptr(
    uo_buf buf)
{
    return ((size_t *)buf)[-2] - ((size_t *)buf)[-1];
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
