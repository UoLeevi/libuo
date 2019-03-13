#include "uo_http_msg.h"
#include "uo_strhashtbl.h"
#include "uo_finstack.h"
#include "uo_mem.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

uo_http_msg *uo_http_msg_create(
    uo_buf *buf)
{
    uo_http_msg *http_msg = calloc(1, sizeof *http_msg);
    http_msg->buf = buf;

    uo_finstack *finstack = http_msg->finstack = uo_finstack_create();
    uo_finstack_add(finstack, finstack, uo_finstack_destroy);

    return http_msg;
}

bool uo_http_msg_set_header(
    uo_http_msg *http_msg,
    const char *header_name,
    const char *header_value)
{
    if (!http_msg->headers)
        http_msg->headers = uo_strhashtbl_create(0x20);

    uo_strhashtbl_set(http_msg->headers, header_name, header_value);

    return true;
}

bool uo_http_msg_set_content(
    uo_http_msg *http_msg,
    const char *content,
    const char *content_type,
    size_t content_len)
{
    http_msg->body = uo_buf_get_len_before_ptr(*http_msg->buf);
    http_msg->body_len = content_len;

    uo_buf_memcpy_append(http_msg->buf, content, content_len);
    uo_buf_null_terminate(http_msg->buf);
    uo_buf_set_ptr_rel(*http_msg->buf, 1);

    int content_len_str_len = uo_buf_printf_append(http_msg->buf, "%lu", content_len);
    char *content_len_str = uo_buf_get_ptr(*http_msg->buf) - content_len_str_len;
    uo_http_msg_set_header(http_msg, "content-length", content_len_str);
    uo_http_msg_set_header(http_msg, "content-type", content_type);
    uo_buf_set_ptr_rel(*http_msg->buf, 1);

    return true;
}

bool uo_http_req_set_request_line(
    uo_http_req *http_req,
    uo_http_method method,
    const char *target,
    uo_http_ver version)
{
    const char *method_str;
    switch (method)
    {
        case UO_HTTP_GET:       method_str = "GET";        break;
        case UO_HTTP_HEAD:      method_str = "HEAD";       break;
        case UO_HTTP_PUT:       method_str = "PUT";        break;
        case UO_HTTP_DELETE:    method_str = "DELETE";     break;
        case UO_HTTP_OPTIONS:   method_str = "OPTIONS";    break;
        case UO_HTTP_TRACE:     method_str = "TRACE";      break;
        case UO_HTTP_POST:      method_str = "POST";       break;
        case UO_HTTP_CONNECT:   method_str = "CONNECT";    break;
        default: return false;
    }

    const char *version_str;
    switch (version)
    {
        case UO_HTTP_1_0:       version_str = UO_HTTP_VER_1_0;          break;
        case UO_HTTP_1_1:       version_str = UO_HTTP_VER_1_1;          break;
        case UO_HTTP_2:         version_str = UO_HTTP_VER_2;            break;
        default: return false;
    }

    if (!*target)
        return false;

    http_req->start_line = uo_buf_get_len_before_ptr(*http_req->buf);
    http_req->start_line_len = uo_buf_printf_append(http_req->buf,
        "%s %s %s", method_str, target, version_str);
    uo_buf_set_ptr_rel(*http_req->buf, 1);

    return uo_http_msg_parse_start_line(http_req);
}

bool uo_http_res_set_status_line(
    uo_http_res *http_res,
    uo_http_status status,
    uo_http_ver version)
{
    const char *status_str;
    switch (status)
    {
        case UO_HTTP_100: status_str = UO_HTTP_STATUS_100; break;
        case UO_HTTP_101: status_str = UO_HTTP_STATUS_101; break;
        case UO_HTTP_200: status_str = UO_HTTP_STATUS_200; break;
        case UO_HTTP_201: status_str = UO_HTTP_STATUS_201; break;
        case UO_HTTP_202: status_str = UO_HTTP_STATUS_202; break;
        case UO_HTTP_203: status_str = UO_HTTP_STATUS_203; break;
        case UO_HTTP_204: status_str = UO_HTTP_STATUS_204; break;
        case UO_HTTP_205: status_str = UO_HTTP_STATUS_205; break;
        case UO_HTTP_206: status_str = UO_HTTP_STATUS_206; break;
        case UO_HTTP_300: status_str = UO_HTTP_STATUS_300; break;
        case UO_HTTP_301: status_str = UO_HTTP_STATUS_301; break;
        case UO_HTTP_302: status_str = UO_HTTP_STATUS_302; break;
        case UO_HTTP_303: status_str = UO_HTTP_STATUS_303; break;
        case UO_HTTP_304: status_str = UO_HTTP_STATUS_304; break;
        case UO_HTTP_305: status_str = UO_HTTP_STATUS_305; break;
        case UO_HTTP_307: status_str = UO_HTTP_STATUS_307; break;
        case UO_HTTP_400: status_str = UO_HTTP_STATUS_400; break;
        case UO_HTTP_401: status_str = UO_HTTP_STATUS_401; break;
        case UO_HTTP_402: status_str = UO_HTTP_STATUS_402; break;
        case UO_HTTP_403: status_str = UO_HTTP_STATUS_403; break;
        case UO_HTTP_404: status_str = UO_HTTP_STATUS_404; break;
        case UO_HTTP_405: status_str = UO_HTTP_STATUS_405; break;
        case UO_HTTP_406: status_str = UO_HTTP_STATUS_406; break;
        case UO_HTTP_407: status_str = UO_HTTP_STATUS_407; break;
        case UO_HTTP_408: status_str = UO_HTTP_STATUS_408; break;
        case UO_HTTP_409: status_str = UO_HTTP_STATUS_409; break;
        case UO_HTTP_410: status_str = UO_HTTP_STATUS_410; break;
        case UO_HTTP_411: status_str = UO_HTTP_STATUS_411; break;
        case UO_HTTP_412: status_str = UO_HTTP_STATUS_412; break;
        case UO_HTTP_413: status_str = UO_HTTP_STATUS_413; break;
        case UO_HTTP_414: status_str = UO_HTTP_STATUS_414; break;
        case UO_HTTP_415: status_str = UO_HTTP_STATUS_415; break;
        case UO_HTTP_416: status_str = UO_HTTP_STATUS_416; break;
        case UO_HTTP_417: status_str = UO_HTTP_STATUS_417; break;
        case UO_HTTP_500: status_str = UO_HTTP_STATUS_500; break;
        case UO_HTTP_501: status_str = UO_HTTP_STATUS_501; break;
        case UO_HTTP_502: status_str = UO_HTTP_STATUS_502; break;
        case UO_HTTP_503: status_str = UO_HTTP_STATUS_503; break;
        case UO_HTTP_504: status_str = UO_HTTP_STATUS_504; break;
        case UO_HTTP_505: status_str = UO_HTTP_STATUS_505; break;
        default: return false;
    }

    const char *version_str;
    switch (version)
    {
        case UO_HTTP_1_0:       version_str = UO_HTTP_VER_1_0;          break;
        case UO_HTTP_1_1:       version_str = UO_HTTP_VER_1_1;          break;
        case UO_HTTP_2:         version_str = UO_HTTP_VER_2;            break;
        default: return false;
    }

    http_res->start_line = uo_buf_get_len_before_ptr(*http_res->buf);
    http_res->start_line_len = uo_buf_printf_append(http_res->buf,
        "%s %s", version_str, status_str);
    uo_buf_set_ptr_rel(*http_res->buf, 1);

    return true;
}

static void uo_http_res_set_content_type_header_based_on_filename(
    uo_http_res *http_res,
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
        case 's': filetype = "image/svg+xml; charset=utf-8"; break;
        default:  filetype = "application/octet-stream"; break;
    }

    if (filetype)
        uo_http_msg_set_header(http_res, "content-type", filetype);
}

bool uo_http_res_set_content_from_file(
    uo_http_res *http_res,
    const char *filename)
{
    struct stat sb;
    if (stat(filename, &sb) == -1 || !S_ISREG(sb.st_mode))
        return false;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return false;

    while (uo_buf_get_len_after_ptr(*http_res->buf) < sb.st_size + 21)
        *http_res->buf = uo_buf_realloc_2x(*http_res->buf);

    if (fread(uo_buf_get_ptr(*http_res->buf), sizeof **http_res->buf, sb.st_size, fp) != sb.st_size || ferror(fp))
        goto err_fclose;

    fclose(fp);

    http_res->body = uo_buf_get_len_before_ptr(*http_res->buf);
    http_res->body_len = sb.st_size;

    uo_buf_set_ptr_rel(*http_res->buf, sb.st_size);
    uo_buf_null_terminate(http_res->buf);
    uo_buf_set_ptr_rel(*http_res->buf, 1);

    int content_len_str_len = uo_buf_printf_append(http_res->buf, "%lu", sb.st_size);
    char *content_len_str = uo_buf_get_ptr(*http_res->buf) - content_len_str_len;
    uo_http_msg_set_header(http_res, "content-length", content_len_str);
    uo_http_res_set_content_type_header_based_on_filename(http_res, filename);
    uo_buf_set_ptr_rel(*http_res->buf, 1);

    return true;

err_fclose:
    fclose(fp);

    return false;
}

bool uo_http_msg_parse_start_line(
    uo_http_msg *http_msg)
{
    uo_buf buf = *http_msg->buf;

    char *start_line = buf + http_msg->start_line;
    size_t len = uo_buf_get_ptr(buf) - (unsigned char *)start_line;

    char *cr = memchr(start_line, '\r', len);
    if (!cr)
        return false;

    *cr = '\0';
    size_t start_ine_len = http_msg->start_line_len = cr - start_line;

    char *sp = memchr(start_line, ' ', start_ine_len);
    if (!sp)
        return false;

    char *target = sp + 1;

    char *target_end = strrchr(target, ' ');
    if (!target_end)
        return false;

    if (http_msg->uri)
        free(http_msg->uri);

    char *uri = http_msg->uri = malloc(target_end - target + 1);

    // decode URI
    // https://stackoverflow.com/a/14530993
    char a, b;
    while (target != target_end)
    {
        if (*target == '+') 
        {
            *uri++ = ' ';
            target++;
        }
        else if ((*target == '%') &&
            ((a = target[1]) && (b = target[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a') 
                a -= 'a' - 'A';

            if (a >= 'A')
                a -= ('A' - 0xA);
            else 
                a -= '0';

            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 0xA);
            else 
                b -= '0';

            *uri++ = 0x10 * a + b;
            target += 3;
        }
        else
            *uri++ = *target++;
    }

    *uri++ = '\0';

    return true;
}

bool uo_http_msg_parse_headers(
    uo_http_msg *http_msg)
{
    uo_buf_null_terminate(http_msg->buf);

    uo_buf buf = *http_msg->buf;
    size_t len = uo_buf_get_len_before_ptr(buf);

    char *end = buf + len;
    char *headers_start = buf + http_msg->start_line_len + UO_STRLEN("\r\n");
    char *headers_end = strstr(headers_start, "\r\n\r\n");

    if (!headers_end)
    {
        // TODO:
        // this changes when start_line parsing changes

        if (strstr(headers_start, "\r\n") == headers_start)
        { 
            // no headers
            http_msg->body = headers_start + UO_STRLEN("\r\n") - (char *)buf;
            http_msg->headers = uo_strhashtbl_create(0);

            return true;
        }

        return false;
    }

    http_msg->body = headers_end + UO_STRLEN("\r\n\r\n") - (char *)buf;

    size_t headers_len = headers_end - headers_start;
    char *headers_buf = malloc(headers_len + 1);
    headers_buf[headers_len] = '\0';
    memcpy(headers_buf, headers_start, headers_len);

    uo_finstack_add(http_msg->finstack, headers_buf, free);

    headers_end = headers_buf + headers_len;

    char *saveptr;
    char *header_name = strtok_r(headers_buf, ":", &saveptr);
    char *header_value;

    uo_strhashtbl *headers = http_msg->headers = uo_strhashtbl_create(0);

    while (header_name)
    {
        char *p = header_name;
        while (*p)
        {
            *p = tolower(*p);
            ++p;
        }

        ++p;

        while ((isspace(*p)))
            ++p;

        header_value = strtok_r(p, "\r", &saveptr);

        uo_strhashtbl_set(headers, header_name, header_value);

        header_name = strtok_r(NULL, "\n:", &saveptr);
    }

    return true;

err_free_headers:
    free(headers);

    http_msg->state.is_invalid = true;

    return false;
}

bool uo_http_msg_parse_body(
    uo_http_msg *http_msg)
{
    uo_buf buf = *http_msg->buf;

    char *header_transfer_encoding = uo_strhashtbl_get(http_msg->headers, "transfer-encoding");
    if (header_transfer_encoding)
    {
        // TODO
    }

    char *header_content_length = uo_strhashtbl_get(http_msg->headers, "content-length");
    if (header_content_length)
    {
        char *endptr;
        errno = 0;
        size_t content_len = strtoull(header_content_length, &endptr, 10);

        if (errno)
            return false;

        buf[http_msg->body + content_len] = '\0';
        http_msg->body_len = content_len;
        
        return true;
    }

    if (*(uint32_t *)(buf + http_msg->start_line) == *(uint32_t *)"HTTP")
        switch (uo_http_res_get_status(http_msg))
        {
            case UO_HTTP_100:
            case UO_HTTP_101:
            case UO_HTTP_204:
            case UO_HTTP_304:
                return true;
        }
    else
        switch (uo_http_req_get_method(http_msg))
        {
            case UO_HTTP_GET:
            case UO_HTTP_TRACE:
                return true;
        }

    return false;
}

uo_http_ver uo_http_msg_get_version(
    uo_http_msg *http_msg)
{
    uo_buf buf = *http_msg->buf;
    char *start_line = buf + http_msg->start_line;
    bool is_response = *(uint32_t *)start_line == *(uint32_t *)"HTTP";
    char last_ver_char;

    if (is_response)
    {
        char *sp = memchr(start_line, ' ', http_msg->start_line_len);
        
        if (!sp)
            return UO_HTTP_VER_INVALID;

        last_ver_char = sp[-1];
    }
    else
        last_ver_char = start_line[http_msg->start_line_len - 1];

    switch (last_ver_char)
    {
        case '0': return UO_HTTP_1_0;
        case '1': return UO_HTTP_1_1;
        case '2': return UO_HTTP_2;
        default:  return UO_HTTP_VER_INVALID;
    }
}

char *uo_http_msg_get_body(
    uo_http_msg *http_msg)
{
    return *http_msg->buf + http_msg->body;
}

char *uo_http_msg_get_header(
    uo_http_msg *http_msg,
    const char *header_name)
{
    return uo_strhashtbl_get(http_msg->headers, header_name);
}

uo_http_method uo_http_req_get_method(
    uo_http_req *http_req)
{
    char *start_line = *http_req->buf + http_req->start_line;
    char *sp = memchr(start_line, ' ', http_req->start_line_len);

    if (!sp)
        return UO_HTTP_METHOD_INVALID;

    switch (sp - start_line)
    {
        case 3:
            if (memcmp(start_line, "GET", 3) == 0) return UO_HTTP_GET;
            if (memcmp(start_line, "PUT", 3) == 0) return UO_HTTP_PUT;
            break;

        case 4:
            if (memcmp(start_line, "POST", 4) == 0) return UO_HTTP_POST;
            if (memcmp(start_line, "HEAD", 4) == 0) return UO_HTTP_HEAD;
            break;

        case 5:
            if (memcmp(start_line, "TRACE", 5) == 0) return UO_HTTP_TRACE;
            break;

        case 6:
            if (memcmp(start_line, "DELETE", 6) == 0) return UO_HTTP_DELETE;
            break;

        case 7:
            if (memcmp(start_line, "OPTIONS", 7) == 0) return UO_HTTP_OPTIONS;
            if (memcmp(start_line, "CONNECT", 7) == 0) return UO_HTTP_CONNECT;
            break;
    }

    return UO_HTTP_METHOD_INVALID;
}

char *uo_http_req_get_uri(
    uo_http_req *http_req)
{
    return http_req->uri;
}

uo_http_status uo_http_res_get_status(
    uo_http_res *http_res)
{
    int status_code;
    if (sscanf(*http_res->buf + http_res->start_line, "%*s %d ", &status_code) != 1)
        return UO_HTTP_STATUS_INVALID;

    switch (status_code)
    {
        case 100: return UO_HTTP_100;
        case 101: return UO_HTTP_101;
        case 200: return UO_HTTP_200;
        case 201: return UO_HTTP_201;
        case 202: return UO_HTTP_202;
        case 203: return UO_HTTP_203;
        case 204: return UO_HTTP_204;
        case 205: return UO_HTTP_205;
        case 206: return UO_HTTP_206;
        case 300: return UO_HTTP_300;
        case 301: return UO_HTTP_301;
        case 302: return UO_HTTP_302;
        case 303: return UO_HTTP_303;
        case 304: return UO_HTTP_304;
        case 305: return UO_HTTP_305;
        case 307: return UO_HTTP_307;
        case 400: return UO_HTTP_400;
        case 401: return UO_HTTP_401;
        case 402: return UO_HTTP_402;
        case 403: return UO_HTTP_403;
        case 404: return UO_HTTP_404;
        case 405: return UO_HTTP_405;
        case 406: return UO_HTTP_406;
        case 407: return UO_HTTP_407;
        case 408: return UO_HTTP_408;
        case 409: return UO_HTTP_409;
        case 410: return UO_HTTP_410;
        case 411: return UO_HTTP_411;
        case 412: return UO_HTTP_412;
        case 413: return UO_HTTP_413;
        case 414: return UO_HTTP_414;
        case 415: return UO_HTTP_415;
        case 416: return UO_HTTP_416;
        case 417: return UO_HTTP_417;
        case 500: return UO_HTTP_500;
        case 501: return UO_HTTP_501;
        case 502: return UO_HTTP_502;
        case 503: return UO_HTTP_503;
        case 504: return UO_HTTP_504;
        case 505: return UO_HTTP_505;
        default:  return UO_HTTP_STATUS_INVALID;
    }
}

void uo_http_msg_write_to_buf(
    uo_http_msg *http_msg,
    uo_buf *dst)
{
    uo_buf buf = *http_msg->buf;
    uo_buf_set_ptr_abs(buf, 0);
    uo_buf_memcpy_append(dst, buf + http_msg->start_line, http_msg->start_line_len);
    uo_buf_memcpy_append(dst, "\r\n", UO_STRLEN("\r\n"));

    if (http_msg->headers)
    {
        uo_strhashtbl *headers = http_msg->headers;
        uo_strkvp_linklist *headers_list = uo_strhashtbl_list(http_msg->headers);
        uo_strkvp_linklist *header_link = uo_strkvp_linklist_next(headers_list);

        while (header_link != headers_list)
        {
            uo_buf_printf_append(dst, "%s: %s\r\n", header_link->item.key, header_link->item.value);
            header_link = uo_strkvp_linklist_next(header_link);
        }
    }

    uo_buf_memcpy_append(dst, "\r\n", UO_STRLEN("\r\n"));

    if (http_msg->body_len)
        uo_buf_memcpy_append(dst, buf + http_msg->body, http_msg->body_len);
}

void uo_http_msg_destroy(
    uo_http_msg *http_msg)
{
    if (http_msg->uri)
        free(http_msg->uri);

    if (http_msg->headers)
        uo_strhashtbl_destroy(http_msg->headers);

    uo_finstack_finalize(http_msg->finstack);

    free(http_msg);
}
