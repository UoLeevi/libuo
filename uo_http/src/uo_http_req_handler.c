#include "uo_http_req_handler.h"
#include "uo_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

uo_http_req_handler *uo_http_req_handler_create(
    const char *req_pattern,
    const uo_cb *cb)
{
    uo_http_req_handler *http_req_handler = calloc(1, sizeof *http_req_handler);
    http_req_handler->cb = uo_cb_clone(cb);

    // allocate copy of req_pattern + extra null terminator;
    size_t req_pattern_len = strlen(req_pattern);
    char *p = http_req_handler->req_pattern = malloc(req_pattern_len + 2);
    memcpy(p, req_pattern, req_pattern_len + 1);
    p[req_pattern_len + 1] = '\0';

    size_t param_count = 0;

    while (p = strchr(p, '{'))
    {
        p = strchr(p, '}');
        assert(p);
        *p++ = '\0';
        ++param_count;
    }

    http_req_handler->param_count = param_count;

    return http_req_handler;
}

void uo_http_req_handler_destroy(
    uo_http_req_handler *http_req_handler)
{
    uo_cb_destroy(http_req_handler->cb);
    free(http_req_handler->req_pattern);
    free(http_req_handler);
}

bool uo_http_req_handler_try(
    uo_http_req_handler *http_req_handler,
    const char *method_sp_uri,
    uo_strhashtbl *params,
    uo_refstack *refstack)
{
    char *req_pattern = http_req_handler->req_pattern;
    size_t i = http_req_handler->param_count;

    if (i == 0)
    {
        // request line is checked for exact match or prefix match
        char *p = uo_strdiff(req_pattern, method_sp_uri);
        return !p || *p == '*';
    }

    // temporary array for pattern parameter name start, value start and value end pointers
    char *param_ptrs[i * 3];

    while (i--)
    {
        // find pattern parameter
        char *param_start = uo_strdiff(req_pattern, method_sp_uri);

        if (*param_start != '{')
            return false;

        // store pattern parameter name pointer
        param_ptrs[i * 3] = param_start + 1;

        // store matched pattern parameter value start pointer
        param_ptrs[i * 3 + 1] = (char *)(method_sp_uri += param_start - req_pattern);

        // skip pattern parameter name
        req_pattern = param_start + strlen(param_start) + 1;

        // find pattern parameter value end
        method_sp_uri = strchr(method_sp_uri, *req_pattern);

        if (!method_sp_uri)
            return false;

        // store matched pattern parameter end pointer
        param_ptrs[i * 3 + 2] = (char *)method_sp_uri;
    }

    {
        // chech if the request line end either a exact match or a prefix match
        char *p = uo_strdiff(req_pattern, method_sp_uri);
        if (p && *p != '*')
            return false;
    }

    // req_handler matches the request method and URI
    // now store the pattern parameters

    i = http_req_handler->param_count;
    char *first_param_value = param_ptrs[(i - 1) * 3 + 1];
    char *param_value_buf = strdup(first_param_value);
    uo_refstack_push(refstack, param_value_buf, free);

    while (i--)
    {
        // null terminate parameter value
        char *param_value = param_value_buf + (param_ptrs[i * 3 + 1] - first_param_value);
        param_value_buf[param_ptrs[i * 3 + 2] - first_param_value] = '\0';

        // get parameter name
        char *param_name = param_ptrs[i * 3];

        // store parameter name and value
        uo_strhashtbl_set(params, param_name, param_value);
    }

    return true;
}
