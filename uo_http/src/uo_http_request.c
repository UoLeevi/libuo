#include "uo_http_request.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uo_http_request *uo_http_request_create(void)
{
    uo_http_request *http_request = malloc(sizeof *http_request);
    return http_request;
}

bool uo_http_request_parse_start_line(
    uo_http_request *http_request,
    uo_buf buf)
{
    // start_line format:
    // {method} {target} {version}\r\n
    //         ^        ^         ^
    //         sp0      sp1       cr

    char *cr = memchr(buf, '\r', uo_buf_get_len_before_ptr(buf));
    if (!cr)
        return false;
    *cr = '\0';
    char *method = buf;

    char *sp0 = memchr(method, ' ', cr - method);
    if (!sp0)
        return false;
    *sp0 = '\0';
    char *target = sp0 + 1;

    char *sp1 = memchr(target, ' ', cr - target);
    if (!sp1)
        return false;
    *sp1 = '\0';
    char *version = sp1 + 1;

    uo_buf_set_ptr_abs(buf, cr - method + 2);

    http_request->start_line.method = method;
    http_request->start_line.target = target;
    http_request->start_line.version = version;

    return true;
}

bool uo_http_request_parse_path(
    uo_http_request *http_request,
    const char *root_dir_path)
{
    char *target = http_request->start_line.target;
    char *path = http_request->path;

    if (strcmp(target, "/") == 0)
        sprintf(path, "%s/index.html", root_dir_path);
    else
    {
        size_t root_dir_path_len = strlen(root_dir_path);
        size_t target_len = strlen(target);

        char *path_end;
        if ((path_end = memchr(target, '?', target_len)) ||
            (path_end = memchr(target, '#', target_len)))
        {
            *path_end = '\0';
            target_len = path_end - target;
        }

        memmove(path + root_dir_path_len, target, target_len + 1);
        memcpy(path, root_dir_path, root_dir_path_len);
    }

    return true;
}

void uo_http_request_destroy(
    uo_http_request *http_request)
{
    free(http_request);
}