#include "uo_http_response.h"

#include <stdlib.h>
#include <string.h>

uo_http_response *uo_http_response_create(void)
{
    uo_http_response *http_response = malloc(sizeof *http_response);
    return http_response;
}

bool uo_http_response_parse_status_line(
    uo_http_response *http_response,
    uo_buf buf)
{
    // status_line format:
    // {version} {status_code} {status_text}\r\n
    //          ^             ^             ^
    //          sp0           sp1           cr

    char *cr = memchr(buf, '\r', uo_buf_get_len_before_ptr(buf));
    if (!cr)
        return false;
    *cr = '\0';
    char *version = buf;

    char *sp0 = memchr(version, ' ', cr - version);
    if (!sp0)
        return false;
    *sp0 = '\0';
    char *status_code = sp0 + 1;

    char *sp1 = memchr(status_code, ' ', cr - status_code);
    if (!sp1)
        return false;
    *sp1 = '\0';
    char *status_text = sp1 + 1;

    uo_buf_set_ptr_abs(buf, cr - version + 2);

    http_response->status_line.version = version;
    http_response->status_line.status_code = status_code;
    http_response->status_line.status_text = status_text;

    return true;
}

void uo_http_response_destroy(
    uo_http_response *http_response)
{
    free(http_response);
}