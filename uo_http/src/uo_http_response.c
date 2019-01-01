#include "uo_http_response.h"
#include "uo_strhashtbl.h"
#include "uo_mem.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <sys/stat.h>

static void uo_http_response_set_content_type_based_on_filename(
    uo_http_response *http_response,
    const char *filename)
{
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types

    const char *file_extension = strrchr(filename, '.');
    if (!file_extension)
        return;

    size_t file_extension_len = strlen(file_extension);
    if (file_extension_len == 1)
        return;

    char *filetype = NULL;
    switch (tolower(file_extension[1]))
    {
        case 'h': filetype = "text/html; charset=utf-8"; break;
        case 'j': filetype = "application/javascript; charset=utf-8"; break;
        case 'c': filetype = "text/css; charset=utf-8"; break;
    }

    if (filetype)
        uo_http_response_set_header(http_response, "content-type", filetype);
}

uo_http_response *uo_http_response_create(void)
{
    uo_http_response *http_response = calloc(1, sizeof *http_response);
    http_response->headers = uo_strhashtbl_create(0x20);
    http_response->buf = uo_buf_alloc(0x200);
    return http_response;
}

bool uo_http_response_set_status(
    uo_http_response *http_response,
    UO_HTTP_STATUS status)
{
    http_response->status = status;
    return true;
}

bool uo_http_response_set_header(
    uo_http_response *http_response,
    const char *header_name,
    char *header_value)
{
    uo_strhashtbl_insert(http_response->headers, header_name, header_value);
    return true;
}

bool uo_http_response_set_content(
    uo_http_response *http_response,
    const char *content,
    char *content_type,
    size_t content_len)
{
    uo_buf_set_ptr_abs(http_response->buf, 0);
    uo_buf_memcpy_append(&http_response->buf, content, content_len);
    int content_len_str_len = uo_buf_printf_append(&http_response->buf, "%lu", content_len);
    char *content_len_str = uo_buf_get_ptr(http_response->buf) - content_len_str_len;
    uo_http_response_set_header(http_response, "content-length", content_len_str);
    uo_http_response_set_header(http_response, "content-type", content_type);
    http_response->content_len = content_len;
    return true;
}

bool uo_http_response_set_file(
    uo_http_response *http_response,
    const char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1 || !S_ISREG(sb.st_mode))
        return false;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return false;

    if (uo_buf_get_size(http_response->buf) < sb.st_size + 21)
        http_response->buf = uo_buf_realloc(http_response->buf, sb.st_size + 21);

    if (fread(http_response->buf, sizeof *http_response->buf, sb.st_size, fp) != sb.st_size || ferror(fp))
        goto err_fclose;

    uo_buf_set_ptr_abs(http_response->buf, sb.st_size);

    fclose(fp);

    int content_len_str_len = uo_buf_printf_append(&http_response->buf, "%lu", sb.st_size);
    char *content_len_str = uo_buf_get_ptr(http_response->buf) - content_len_str_len;
    uo_http_response_set_header(http_response, "content-length", content_len_str);
    http_response->content_len = sb.st_size;

    uo_http_response_set_content_type_based_on_filename(http_response, filename);

    return true;

err_fclose:
    fclose(fp);

    return false;
}

void uo_http_response_destroy(
    uo_http_response *http_response)
{
    uo_strhashtbl_destroy(http_response->headers);
    uo_buf_free(http_response->buf);
    free(http_response);
}