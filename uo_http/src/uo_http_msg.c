#include "uo_http_msg.h"
#include "uo_hashtbl.h"
#include "uo_finstack.h"
#include "uo_mem.h"

#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

void uo_http_msg_create_at(
    uo_http_msg *http_msg,
    uo_buf *buf,
    uo_http_msg_type type,
    uo_http_msg_role role)
{
    http_msg->buf = buf;
    http_msg->flags.type = type;
    http_msg->flags.role = role;

    uo_strhashtbl_create_at(&http_msg->headers, 0);

    uo_finstack *finstack = http_msg->finstack = uo_finstack_create();
    uo_finstack_add(finstack, finstack, (void (*)(void *))uo_finstack_destroy);
}

void uo_http_msg_destroy_at(
    uo_http_msg *http_msg)
{
    uo_strhashtbl_destroy_at(&http_msg->headers);
    uo_finstack_finalize(http_msg->finstack);
}

bool uo_http_msg_set_header(
    uo_http_msg *http_msg,
    const char *header_name,
    const char *header_value)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_SEND);
    uo_strhashtbl_set(&http_msg->headers, header_name, header_value);
    return true;
}

bool uo_http_msg_set_content(
    uo_http_msg *http_msg,
    const char *content,
    const char *content_type,
    size_t content_len)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_SEND);

    if (!content_len)
    {
        uo_http_msg_set_header(http_msg, "content-length", "0");
        return http_msg->flags.body = true;
    }

    size_t content_len_str_len = snprintf(NULL, 0, "%lu", content_len);
    size_t content_type_len = strlen(content_type);

    http_msg->body_len = content_len;

    char *p = http_msg->body = malloc(content_len + content_len_str_len + content_type_len + 3);
    uo_finstack_add(http_msg->finstack, p, free);

    memcpy(p, content, content_len);
    p += content_len;
    *p++ = '\0';

    sprintf(p, "%lu", content_len);
    uo_http_msg_set_header(http_msg, "content-length", p);
    p += content_len_str_len + 1;

    memcpy(p, content_type, content_type_len);
    p += content_type_len;
    *p++ = '\0';

    return http_msg->flags.body = true;
}

bool uo_http_req_set_request_line(
    uo_http_req *http_req,
    uo_http_method method,
    const char *uri,
    uo_http_ver version)
{
    assert(http_req->flags.role == UO_HTTP_MSG_ROLE_SEND);

    const char *method_str;
    size_t method_str_len;
    switch (method)
    {
        case UO_HTTP_GET:     method_str = "GET";     method_str_len = 3; break;
        case UO_HTTP_HEAD:    method_str = "HEAD";    method_str_len = 4; break;
        case UO_HTTP_PUT:     method_str = "PUT";     method_str_len = 3; break;
        case UO_HTTP_DELETE:  method_str = "DELETE";  method_str_len = 6; break;
        case UO_HTTP_OPTIONS: method_str = "OPTIONS"; method_str_len = 7; break;
        case UO_HTTP_TRACE:   method_str = "TRACE";   method_str_len = 5; break;
        case UO_HTTP_POST:    method_str = "POST";    method_str_len = 4; break;
        case UO_HTTP_CONNECT: method_str = "CONNECT"; method_str_len = 7; break;
        default: return false;
    }

    const char *version_str;
    size_t version_str_len;
    switch (version)
    {
        case UO_HTTP_VER_1_0: version_str = "HTTP/1.0"; version_str_len = 8; break;
        case UO_HTTP_VER_1_1: version_str = "HTTP/1.1"; version_str_len = 8; break;
        case UO_HTTP_VER_2:   version_str = "HTTP/2";   version_str_len = 6; break;
        default: return false;
    }

    if (!*uri)
        return false;

    size_t uri_len = strlen(uri);

    char *p = malloc(method_str_len + uri_len + version_str_len + 5);
    uo_finstack_add(http_req->finstack, p, free);

    http_req->method_sp_uri = memcpy(p, method_str, method_str_len);
    p += method_str_len;
    *p++ = ' ';

    http_req->uri = memcpy(p, uri, uri_len);
    p += uri_len;
    *p++ = '\0'; // replaced with ' ' before sending the message

    http_req->version_crlf = memcpy(p, version_str, version_str_len);
    p += version_str_len;
    *p++ = '\r';
    *p++ = '\n';
    *p++ = '\0';

    http_req->ver = version;
    http_req->method = method;

    return http_req->flags.start_line = true;
}

bool uo_http_res_set_status_line(
    uo_http_res *http_res,
    uo_http_status status,
    uo_http_ver version)
{
    assert(http_res->flags.role == UO_HTTP_MSG_ROLE_SEND);

    const char *status_str;
    size_t status_str_len;
    switch (status)
    {
        case UO_HTTP_100: status_str = UO_HTTP_STATUS_100; status_str_len = UO_STRLEN(UO_HTTP_STATUS_100); break;
        case UO_HTTP_101: status_str = UO_HTTP_STATUS_101; status_str_len = UO_STRLEN(UO_HTTP_STATUS_101); break;
        case UO_HTTP_200: status_str = UO_HTTP_STATUS_200; status_str_len = UO_STRLEN(UO_HTTP_STATUS_200); break;
        case UO_HTTP_201: status_str = UO_HTTP_STATUS_201; status_str_len = UO_STRLEN(UO_HTTP_STATUS_201); break;
        case UO_HTTP_202: status_str = UO_HTTP_STATUS_202; status_str_len = UO_STRLEN(UO_HTTP_STATUS_202); break;
        case UO_HTTP_203: status_str = UO_HTTP_STATUS_203; status_str_len = UO_STRLEN(UO_HTTP_STATUS_203); break;
        case UO_HTTP_204: status_str = UO_HTTP_STATUS_204; status_str_len = UO_STRLEN(UO_HTTP_STATUS_204); break;
        case UO_HTTP_205: status_str = UO_HTTP_STATUS_205; status_str_len = UO_STRLEN(UO_HTTP_STATUS_205); break;
        case UO_HTTP_206: status_str = UO_HTTP_STATUS_206; status_str_len = UO_STRLEN(UO_HTTP_STATUS_206); break;
        case UO_HTTP_300: status_str = UO_HTTP_STATUS_300; status_str_len = UO_STRLEN(UO_HTTP_STATUS_300); break;
        case UO_HTTP_301: status_str = UO_HTTP_STATUS_301; status_str_len = UO_STRLEN(UO_HTTP_STATUS_301); break;
        case UO_HTTP_302: status_str = UO_HTTP_STATUS_302; status_str_len = UO_STRLEN(UO_HTTP_STATUS_302); break;
        case UO_HTTP_303: status_str = UO_HTTP_STATUS_303; status_str_len = UO_STRLEN(UO_HTTP_STATUS_303); break;
        case UO_HTTP_304: status_str = UO_HTTP_STATUS_304; status_str_len = UO_STRLEN(UO_HTTP_STATUS_304); break;
        case UO_HTTP_305: status_str = UO_HTTP_STATUS_305; status_str_len = UO_STRLEN(UO_HTTP_STATUS_305); break;
        case UO_HTTP_307: status_str = UO_HTTP_STATUS_307; status_str_len = UO_STRLEN(UO_HTTP_STATUS_307); break;
        case UO_HTTP_400: status_str = UO_HTTP_STATUS_400; status_str_len = UO_STRLEN(UO_HTTP_STATUS_400); break;
        case UO_HTTP_401: status_str = UO_HTTP_STATUS_401; status_str_len = UO_STRLEN(UO_HTTP_STATUS_401); break;
        case UO_HTTP_402: status_str = UO_HTTP_STATUS_402; status_str_len = UO_STRLEN(UO_HTTP_STATUS_402); break;
        case UO_HTTP_403: status_str = UO_HTTP_STATUS_403; status_str_len = UO_STRLEN(UO_HTTP_STATUS_403); break;
        case UO_HTTP_404: status_str = UO_HTTP_STATUS_404; status_str_len = UO_STRLEN(UO_HTTP_STATUS_404); break;
        case UO_HTTP_405: status_str = UO_HTTP_STATUS_405; status_str_len = UO_STRLEN(UO_HTTP_STATUS_405); break;
        case UO_HTTP_406: status_str = UO_HTTP_STATUS_406; status_str_len = UO_STRLEN(UO_HTTP_STATUS_406); break;
        case UO_HTTP_407: status_str = UO_HTTP_STATUS_407; status_str_len = UO_STRLEN(UO_HTTP_STATUS_407); break;
        case UO_HTTP_408: status_str = UO_HTTP_STATUS_408; status_str_len = UO_STRLEN(UO_HTTP_STATUS_408); break;
        case UO_HTTP_409: status_str = UO_HTTP_STATUS_409; status_str_len = UO_STRLEN(UO_HTTP_STATUS_409); break;
        case UO_HTTP_410: status_str = UO_HTTP_STATUS_410; status_str_len = UO_STRLEN(UO_HTTP_STATUS_410); break;
        case UO_HTTP_411: status_str = UO_HTTP_STATUS_411; status_str_len = UO_STRLEN(UO_HTTP_STATUS_411); break;
        case UO_HTTP_412: status_str = UO_HTTP_STATUS_412; status_str_len = UO_STRLEN(UO_HTTP_STATUS_412); break;
        case UO_HTTP_413: status_str = UO_HTTP_STATUS_413; status_str_len = UO_STRLEN(UO_HTTP_STATUS_413); break;
        case UO_HTTP_414: status_str = UO_HTTP_STATUS_414; status_str_len = UO_STRLEN(UO_HTTP_STATUS_414); break;
        case UO_HTTP_415: status_str = UO_HTTP_STATUS_415; status_str_len = UO_STRLEN(UO_HTTP_STATUS_415); break;
        case UO_HTTP_416: status_str = UO_HTTP_STATUS_416; status_str_len = UO_STRLEN(UO_HTTP_STATUS_416); break;
        case UO_HTTP_417: status_str = UO_HTTP_STATUS_417; status_str_len = UO_STRLEN(UO_HTTP_STATUS_417); break;
        case UO_HTTP_500: status_str = UO_HTTP_STATUS_500; status_str_len = UO_STRLEN(UO_HTTP_STATUS_500); break;
        case UO_HTTP_501: status_str = UO_HTTP_STATUS_501; status_str_len = UO_STRLEN(UO_HTTP_STATUS_501); break;
        case UO_HTTP_502: status_str = UO_HTTP_STATUS_502; status_str_len = UO_STRLEN(UO_HTTP_STATUS_502); break;
        case UO_HTTP_503: status_str = UO_HTTP_STATUS_503; status_str_len = UO_STRLEN(UO_HTTP_STATUS_503); break;
        case UO_HTTP_504: status_str = UO_HTTP_STATUS_504; status_str_len = UO_STRLEN(UO_HTTP_STATUS_504); break;
        case UO_HTTP_505: status_str = UO_HTTP_STATUS_505; status_str_len = UO_STRLEN(UO_HTTP_STATUS_505); break;
        default: return false;
    }

    const char *version_str;
    size_t version_str_len;
    switch (version)
    {
        case UO_HTTP_VER_1_0: version_str = "HTTP/1.0"; version_str_len = 8; break;
        case UO_HTTP_VER_1_1: version_str = "HTTP/1.1"; version_str_len = 8; break;
        case UO_HTTP_VER_2:   version_str = "HTTP/2";   version_str_len = 6; break;
        default: return false;
    }

    char *p = malloc(version_str_len + status_str_len + 4);
    uo_finstack_add(http_res->finstack, p, free);

    http_res->version = memcpy(p, version_str, version_str_len);
    p += version_str_len;
    *p++ = '\0'; // replaced with ' ' before sending the message

    http_res->status_sp_reason = memcpy(p, status_str, status_str_len);
    p += status_str_len;
    *p++ = '\0'; // replaced with '\r' before sending the message
    *p++ = '\n';
    *p++ = '\0';

    http_res->ver = version;
    http_res->status = status;

    return http_res->flags.start_line = true;
}

static void uo_http_res_set_content_type_header_based_on_filename(
    uo_http_res *http_res,
    const char *filename)
{
    assert(http_res->flags.role == UO_HTTP_MSG_ROLE_SEND);

    const char *file_extension = strrchr(filename, '.');
    if (!file_extension)
        return;

    size_t file_extension_len = strlen(file_extension);
    if (file_extension_len == 1)
        return;

    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
    char *filetype = NULL;
    switch (tolower(file_extension[1]))
    {
        case 'h': filetype = "text/html; charset=utf-8";              break;
        case 'j': filetype = "application/javascript; charset=utf-8"; break;
        case 'c': filetype = "text/css; charset=utf-8";               break;
        case 's': filetype = "image/svg+xml; charset=utf-8";          break;
        default:  filetype = "application/octet-stream";              break;
    }

    if (filetype)
        uo_http_msg_set_header(http_res, "content-type", filetype);
}

bool uo_http_res_set_content_from_file(
    uo_http_res *http_res,
    const char *filename)
{
    assert(http_res->flags.role == UO_HTTP_MSG_ROLE_SEND);

    struct stat sb;
    if (stat(filename, &sb) == -1 || !S_ISREG(sb.st_mode))
        return false;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return false;

    size_t content_len_str_len = snprintf(NULL, 0, "%lu", sb.st_size);

    char *p = malloc(sb.st_size + content_len_str_len + 2);
    uo_finstack_add(http_res->finstack, p, free);

    if (fread(p, sizeof *p, sb.st_size, fp) != sb.st_size || ferror(fp))
        goto err_fclose;

    fclose(fp);

    http_res->body = p;
    http_res->body_len = sb.st_size;
    p += sb.st_size;
    *p++ = '\0';

    sprintf(p, "%lu", sb.st_size);
    uo_http_msg_set_header(http_res, "content-length", p);

    uo_http_res_set_content_type_header_based_on_filename(http_res, filename);

    return http_res->flags.body = true;

err_fclose:
    fclose(fp);

    return false;
}

bool uo_http_msg_parse_start_line(
    uo_http_msg *http_msg)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_RECV);

    uo_buf buf = *http_msg->buf;
    char *p = buf;
    size_t len = uo_buf_get_len_before_ptr(buf);

    char *cr = memchr(p, '\r', len - 1);
    if (!cr || cr[1] != '\n')
        return false;

    size_t start_line_len = cr - p + 2;

    char *start_line = malloc(start_line_len + 1);
    uo_finstack_add(http_msg->finstack, start_line, free);

    p = memcpy(start_line, p, start_line_len);
    start_line[start_line_len] = '\0';

    if (http_msg->flags.type == UO_HTTP_MSG_TYPE_REQUEST)
    {
        char *sp1 = memchr(p, ' ', start_line_len);

        if (!sp1)
            return (http_msg->flags.is_invalid = true, false);

        switch (sp1 - p)
        {
            case 3:
                if (memcmp(start_line, "GET", 3) == 0) 
                    http_msg->method = UO_HTTP_GET;
                else if (memcmp(start_line, "PUT", 3) == 0) 
                    http_msg->method = UO_HTTP_PUT;
                break;

            case 4:
                if (memcmp(start_line, "POST", 4) == 0) 
                    http_msg->method = UO_HTTP_POST;
                else if (memcmp(start_line, "HEAD", 4) == 0) 
                    http_msg->method = UO_HTTP_HEAD;
                break;

            case 5:
                if (memcmp(start_line, "TRACE", 5) == 0)
                    http_msg->method = UO_HTTP_TRACE;
                break;

            case 6:
                if (memcmp(start_line, "DELETE", 6) == 0)
                    http_msg->method = UO_HTTP_DELETE;
                break;

            case 7:
                if (memcmp(start_line, "OPTIONS", 7) == 0) 
                    http_msg->method = UO_HTTP_OPTIONS;
                else if (memcmp(start_line, "CONNECT", 7) == 0) 
                    http_msg->method = UO_HTTP_CONNECT;
                break;
        }

        char *sp2 = p = strrchr(sp1 + 1, ' ');

        if (!p)
            return (http_msg->flags.is_invalid = true, false);
        
        *p++ = '\0';
        p = strchr(p, '\r');

        if (!p)
            return (http_msg->flags.is_invalid = true, false);
        
        switch (p[-1])
        {
            case '2': http_msg->ver = UO_HTTP_VER_2;   break; // HTTP/2
            case '1': http_msg->ver = UO_HTTP_VER_1_1; break; // HTTP/1.1
            case '0': http_msg->ver = UO_HTTP_VER_1_0; break; // HTTP/1.0
            default: return (http_msg->flags.is_invalid = true, false);
        }

        http_msg->method_sp_uri = start_line;
        http_msg->uri = sp1 + 1;
        http_msg->version_crlf = sp2 + 1;
    }
    else // http_msg->flags.type == UO_HTTP_MSG_TYPE_RESPONSE
    {
        p[start_line_len - 2] = '\0';
        char *sp1 = p = strchr(p, ' ');
        if (!p)
            return (http_msg->flags.is_invalid = true, false);
        
        switch (p[-1])
        {
            case '2': http_msg->ver = UO_HTTP_VER_2;   break; // HTTP/2
            case '1': http_msg->ver = UO_HTTP_VER_1_1; break; // HTTP/1.1
            case '0': http_msg->ver = UO_HTTP_VER_1_0; break; // HTTP/1.0
            default: return (http_msg->flags.is_invalid = true, false);
        }

        ++p;

        int status_code;
        if (sscanf(p, "%d ", &status_code) != 1)
            return (http_msg->flags.is_invalid = true, false);

        switch (status_code)
        {
            case 100: http_msg->status = UO_HTTP_100; break;
            case 101: http_msg->status = UO_HTTP_101; break;
            case 200: http_msg->status = UO_HTTP_200; break;
            case 201: http_msg->status = UO_HTTP_201; break;
            case 202: http_msg->status = UO_HTTP_202; break;
            case 203: http_msg->status = UO_HTTP_203; break;
            case 204: http_msg->status = UO_HTTP_204; break;
            case 205: http_msg->status = UO_HTTP_205; break;
            case 206: http_msg->status = UO_HTTP_206; break;
            case 300: http_msg->status = UO_HTTP_300; break;
            case 301: http_msg->status = UO_HTTP_301; break;
            case 302: http_msg->status = UO_HTTP_302; break;
            case 303: http_msg->status = UO_HTTP_303; break;
            case 304: http_msg->status = UO_HTTP_304; break;
            case 305: http_msg->status = UO_HTTP_305; break;
            case 307: http_msg->status = UO_HTTP_307; break;
            case 400: http_msg->status = UO_HTTP_400; break;
            case 401: http_msg->status = UO_HTTP_401; break;
            case 402: http_msg->status = UO_HTTP_402; break;
            case 403: http_msg->status = UO_HTTP_403; break;
            case 404: http_msg->status = UO_HTTP_404; break;
            case 405: http_msg->status = UO_HTTP_405; break;
            case 406: http_msg->status = UO_HTTP_406; break;
            case 407: http_msg->status = UO_HTTP_407; break;
            case 408: http_msg->status = UO_HTTP_408; break;
            case 409: http_msg->status = UO_HTTP_409; break;
            case 410: http_msg->status = UO_HTTP_410; break;
            case 411: http_msg->status = UO_HTTP_411; break;
            case 412: http_msg->status = UO_HTTP_412; break;
            case 413: http_msg->status = UO_HTTP_413; break;
            case 414: http_msg->status = UO_HTTP_414; break;
            case 415: http_msg->status = UO_HTTP_415; break;
            case 416: http_msg->status = UO_HTTP_416; break;
            case 417: http_msg->status = UO_HTTP_417; break;
            case 500: http_msg->status = UO_HTTP_500; break;
            case 501: http_msg->status = UO_HTTP_501; break;
            case 502: http_msg->status = UO_HTTP_502; break;
            case 503: http_msg->status = UO_HTTP_503; break;
            case 504: http_msg->status = UO_HTTP_504; break;
            case 505: http_msg->status = UO_HTTP_505; break;
            default: return (http_msg->flags.is_invalid = true, false);
        }
        http_msg->version = start_line;
        http_msg->status_sp_reason = sp1 + 1;
    }

    http_msg->temp.parsing_offset = cr + 2 - (char *)buf;
    return http_msg->flags.start_line = true;
}

bool uo_http_msg_parse_headers(
    uo_http_msg *http_msg)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_RECV);
    uo_buf_null_terminate(http_msg->buf);

    uo_buf buf = *http_msg->buf;
    size_t len = uo_buf_get_len_before_ptr(buf);

    char *end = buf + len;
    char *headers_start = buf + http_msg->temp.parsing_offset;
    char *headers_end = strstr(headers_start - UO_STRLEN("\r\n"), "\r\n\r\n");

    if (!headers_end)
        return false;

    http_msg->temp.parsing_offset = headers_end + UO_STRLEN("\r\n\r\n") - (char *)buf;

    // check if there are no headers
    if (headers_start > headers_end)
        return http_msg->flags.headers = true;

    size_t headers_len = headers_end - headers_start;

    char *p = malloc(headers_len + 1);
    uo_finstack_add(http_msg->finstack, p, free);

    p[headers_len] = '\0';
    memcpy(p, headers_start, headers_len);

    headers_end = p + headers_len;

    char *saveptr;
    char *header_name = strtok_r(p, ":", &saveptr);
    char *header_value;

    uo_strhashtbl *headers = &http_msg->headers;

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

    return http_msg->flags.headers = true;
}

bool uo_http_msg_parse_body(
    uo_http_msg *http_msg)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_RECV);
    uo_buf buf = *http_msg->buf;
    char *body = buf + http_msg->temp.parsing_offset;
    size_t len = (char *)uo_buf_get_ptr(buf) - body;

    uo_strhashtbl *headers = &http_msg->headers;

    char *header_transfer_encoding = uo_strhashtbl_get(headers, "transfer-encoding");
    if (header_transfer_encoding)
    {
        // TODO
    }

    char *header_content_length = uo_strhashtbl_get(headers, "content-length");
    if (header_content_length)
    {
        char *endptr;
        errno = 0;
        size_t content_len = strtoull(header_content_length, &endptr, 10);

        if (errno)
            return (http_msg->flags.is_invalid = true, false);

        if (content_len > len)
            return false;

        http_msg->body = body;
        body[content_len] = '\0';
        http_msg->body_len = content_len;
        
        http_msg->temp.parsing_offset = 0;
        return http_msg->flags.body = true;
    }

    if (http_msg->flags.type == UO_HTTP_MSG_TYPE_REQUEST)
        switch (http_msg->method)
        {
            case UO_HTTP_GET:
            case UO_HTTP_HEAD:
            case UO_HTTP_DELETE:
            case UO_HTTP_CONNECT:
            case UO_HTTP_TRACE:
                http_msg->temp.parsing_offset = 0;
                return http_msg->flags.body = true;
        }
    else // http_msg->flags.type == UO_HTTP_MSG_TYPE_RESPONSE
        switch (http_msg->status)
        {
            case UO_HTTP_100:
            case UO_HTTP_101:
            case UO_HTTP_204:
            case UO_HTTP_304:
                http_msg->temp.parsing_offset = 0;
                return http_msg->flags.body = true;
        }

    return (http_msg->flags.is_invalid = true, false);
}

char *uo_http_msg_get_header(
    uo_http_msg *http_msg,
    const char *header_name)
{
    return uo_strhashtbl_get(&http_msg->headers, header_name);
}

void uo_http_msg_write_to_buf(
    uo_http_msg *http_msg,
    uo_buf *dst)
{
    assert(http_msg->flags.role == UO_HTTP_MSG_ROLE_SEND);
    uo_buf buf = *http_msg->buf;
    uo_buf_set_ptr_abs(buf, 0);

    if (http_msg->flags.type == UO_HTTP_MSG_TYPE_REQUEST)
        uo_buf_printf_append(dst, "%s %s", http_msg->method_sp_uri, http_msg->version_crlf);
    else // http_msg->flags.type == UO_HTTP_MSG_TYPE_RESPONSE
        uo_buf_printf_append(dst, "%s %s\r\n", http_msg->version, http_msg->status_sp_reason);

    uo_strhashtbl *headers = &http_msg->headers;

    if (headers->count)
    {
        uo_strkvp_linklist *headers_list = uo_strhashtbl_list(headers);
        uo_strkvp_linklist *header_link = uo_strkvp_linklist_next(headers_list);

        while (header_link != headers_list)
        {
            uo_buf_printf_append(dst, "%s: %s\r\n", header_link->item.key, header_link->item.value);
            header_link = uo_strkvp_linklist_next(header_link);
        }
    }

    uo_buf_memcpy_append(dst, "\r\n", UO_STRLEN("\r\n"));

    if (http_msg->body_len)
        uo_buf_memcpy_append(dst, http_msg->body, http_msg->body_len);
}
