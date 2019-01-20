#include "uo_http_request.h"
#include "uo_strhashtbl.h"
#include "uo_mem.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

uo_http_request *uo_http_request_create(
    uo_buf *buf)
{
    uo_http_request *http_request = calloc(1, sizeof *http_request);
    http_request->headers = uo_strhashtbl_create(0x20); // TODO: make size dynamic
    http_request->buf = buf;
    return http_request;
}

bool uo_http_request_set_method(
    uo_http_request *http_request,
    UO_HTTP_METHOD method)
{
    http_request->method = method;
    return true;
}

bool uo_http_request_set_target(
    uo_http_request *http_request,
    const char *target)
{
    // TODO
}

bool uo_http_request_set_header(
    uo_http_request *http_request,
    const char *header_name,
    char *header_value)
{
    uo_strhashtbl_insert(http_request->headers, header_name, header_value);
    return true;
}

bool uo_http_request_set_content(
    uo_http_request *http_request,
    const char *content,
    char *content_type,
    size_t content_len)
{
    uo_buf_set_ptr_abs(*http_request->buf, 0);
    uo_buf_memcpy_append(http_request->buf, content, content_len);
    int content_len_str_len = uo_buf_printf_append(http_request->buf, "%lu", content_len);
    char *content_len_str = uo_buf_get_ptr(*http_request->buf) - content_len_str_len;
    uo_http_request_set_header(http_request, "content-length", content_len_str);
    uo_http_request_set_header(http_request, "content-type", content_type);
    http_request->content_len = content_len;
    return true;
}

bool uo_http_request_parse_start_line(
    uo_http_request *http_request)
{
    size_t target;
    uo_buf buf = *http_request->buf;

    switch (http_request->method = uo_http_method_parse(buf))
    {
        case UO_HTTP_1_1_METHOD_GET:
        case UO_HTTP_1_1_METHOD_PUT:
            http_request->target = target = 4;
            break;

        case UO_HTTP_1_1_METHOD_HEAD:
        case UO_HTTP_1_1_METHOD_POST:
            http_request->target = target = 5;
            break;
            
        case UO_HTTP_1_1_METHOD_TRACE:
            http_request->target = target = 6;
            break;
            
        case UO_HTTP_1_1_METHOD_DELETE:
            http_request->target = target = 7;
            break;
            
        case UO_HTTP_1_1_METHOD_OPTIONS:
        case UO_HTTP_1_1_METHOD_CONNECT:
            http_request->target = target = 8;
            break;
            
        default:
            return false;
    }

    char *target_end = strchr(buf + target, ' ');
    if (target_end)
        *target_end = '\0';
    
    return true;
}

bool uo_http_request_parse_headers(
    uo_http_request *http_request)
{
    uo_buf buf = *http_request->buf;
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

    http_request->body = headers_end + UO_STRLEN("\r\n\r\n") - (char *)buf;
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

    http_request->headers = headers;

    return true;

err_free_headers:
    free(headers);

    return false;
}

bool uo_http_request_parse_body(
    uo_http_request *http_request)
{
    uo_buf buf = *http_request->buf;
    char *header_content_length = uo_strhashtbl_find(http_request->headers, "content-length");
    if (header_content_length)
    {
        char *endptr;
        size_t content_len = http_request->content_len = strtoull(header_content_length, &endptr, 10);

        if (!content_len)
            return false;

        buf[http_request->body + content_len] = '\0';
        
        return true;
    }
    
    char *header_transfer_encoding = uo_strhashtbl_find(http_request->headers, "transfer-encoding");
    if (header_transfer_encoding)
    {
        // TODO
    }

    return false;
}

char *uo_http_request_get_target(
    uo_http_request *http_request)
{
    return *http_request->buf + http_request->target;
}

char *uo_http_request_get_body(
    uo_http_request *http_request)
{
    return *http_request->buf + http_request->body;
}


void uo_http_request_destroy(
    uo_http_request *http_request)
{
    uo_strhashtbl_destroy(http_request->headers);
    free(http_request);
}