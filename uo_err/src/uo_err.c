#include "uo_err.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#endif

void uo_print_err(
    const char *fmt,
    va_list args)
{
#ifdef _WIN32
    DWORD winerr = GetLastError();
#endif
    int errno_local = errno;

    if (fmt) 
    {
        vfprintf(stderr, fmt, args);
        fputc('\n', stderr);
    }

    if (errno_local)
        fprintf(stderr, "(errno = %d) : %s\n", errno_local, strerror(errno_local));

#ifdef _WIN32
    if (winerr)
    {
        char buf[256];
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            winerr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf, 
            (sizeof buf / sizeof *buf),
            NULL);
        fprintf(stderr, "(GetLastError() = %d) : %s\n", winerr, buf);
    }
#endif

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
