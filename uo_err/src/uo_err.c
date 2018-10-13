#include "uo_err.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void uo_print_err(
    const char *fmt,
    va_list args)
{
    int errno_local = errno;

    if (fmt) 
    {
        vfprintf(stderr, fmt, args);
        fputc('\n', stderr);
    }

    if (errno_local != 0)
        fprintf(stderr, "(errno = %d) : %s\n", errno_local, strerror(errno_local));

    fflush(stderr);
}

void uo_err(
    const char *fmt,
    ...)
{
    va_list args;
    va_start(args, fmt);
    uo_print_err(fmt, args);
    va_end(args);
}

void uo_err_exit(
    const char *fmt,
    ...)
{
    va_list args;
    va_start(args, fmt);
    uo_print_err(fmt, args);
    va_end(args);
    exit(1);
}
