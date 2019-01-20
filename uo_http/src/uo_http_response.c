#include "uo_http_response.h"
#include "uo_strhashtbl.h"
#include "uo_mem.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
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

uo_http_response *uo_http_response_create(
    uo_buf *buf)
{
    uo_http_response *http_response = calloc(1, sizeof *http_response);
    http_response->headers = uo_strhashtbl_create(0x20); // TODO: make size dynamic
    http_response->buf = buf;
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
    uo_buf_set_ptr_abs(*http_response->buf, 0);
    uo_buf_memcpy_append(http_response->buf, content, content_len);
    int content_len_str_len = uo_buf_printf_append(http_response->buf, "%lu", content_len);
    char *content_len_str = uo_buf_get_ptr(*http_response->buf) - content_len_str_len;
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

    if (uo_buf_get_size(*http_response->buf) < sb.st_size + 21)
        *http_response->buf = uo_buf_realloc(*http_response->buf, sb.st_size + 21);

    if (fread(*http_response->buf, sizeof **http_response->buf, sb.st_size, fp) != sb.st_size || ferror(fp))
        goto err_fclose;

    uo_buf_set_ptr_abs(*http_response->buf, sb.st_size);

    fclose(fp);

    int content_len_str_len = uo_buf_printf_append(http_response->buf, "%lu", sb.st_size);
    char *content_len_str = uo_buf_get_ptr(*http_response->buf) - content_len_str_len;
    uo_http_response_set_header(http_response, "content-length", content_len_str);
    http_response->content_len = sb.st_size;

    uo_http_response_set_content_type_based_on_filename(http_response, filename);

    return true;

err_fclose:
    fclose(fp);

    return false;
}

bool uo_http_response_parse_status_line(
    uo_http_response *http_response)
{
    return (http_response->status = uo_http_status_parse(*http_response->buf)) != UO_HTTP_STATUS_INVALID;
}

bool uo_http_response_parse_headers(
    uo_http_response *http_response)
{
    uo_buf buf = *http_response->buf;
    size_t len = uo_buf_get_len_before_ptr(buf); 
    size_t header_count = 0;
    char *cr = buf;
    char *headers_end = NULL;

    while (!headers_end && (cr = memchr(cr + 1, '\r', len - (cr + 1 - (char *)buf))))
        if (*(uint32_t *)cr == *(uint32_t *)"\r\n\r\n")
            headers_end = cr;
        else
            ++header_count;

    if (!headers_end)
        return false;

    http_response->body = headers_end + UO_STRLEN("\r\n\r\n") - (char *)buf;
    uo_strhashtbl *headers = uo_strhashtbl_create(header_count * 1.5);

    cr = memchr(buf, '\r', headers_end - (char *)buf);

    for (size_t i = 0; i < header_count; ++i)
    {
        char *header_name = cr + 2;
        
        cr = memchr(cr + 1, '\r', headers_end - cr);
        *cr = '\0';
        
        char *header_name_end = memchr(header_name, ':', cr - header_name);
        if (!header_name_end)
            goto err_free_headers;
        
        *header_name_end = '\0';
        
        char *p = header_name_end;
        while (p-- > header_name)
            *p = tolower(*p);

        p = header_name_end + 1;
        while (*p == ' ')
            ++p;
        
        uo_strhashtbl_insert(headers, header_name, p);
    }

    http_response->headers = headers;

    return true;

err_free_headers:
    free(headers);

    return false;
}

bool uo_http_response_parse_body(
    uo_http_response *http_response)
{
    uo_buf buf = *http_response->buf;
    char *header_content_length = uo_strhashtbl_find(http_response->headers, "content-length");
    if (header_content_length)
    {
        char *endptr;
        size_t content_len = http_response->content_len = strtoull(header_content_length, &endptr, 10);

        if (!content_len)
            return false;

        buf[http_response->body + content_len] = '\0';
        
        return true;
    }
    
    char *header_transfer_encoding = uo_strhashtbl_find(http_response->headers, "transfer-encoding");
    if (header_transfer_encoding)
    {
        // TODO
    }

    return false;
}

char *uo_http_response_get_body(
    uo_http_response *http_response)
{
    return *http_response->buf + http_response->body;
}

void uo_http_response_destroy(
    uo_http_response *http_response)
{
    uo_strhashtbl_destroy(http_response->headers);
    free(http_response);
}