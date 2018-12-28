#ifndef UO_HTTP_RESPONSE_H
#define UO_HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uo_buf.h"

#include <stdbool.h>

typedef struct uo_http_response
{
    struct
    {
        char *version;
        char *status_code;
        char *status_text;
    } status_line;

} uo_http_response;

uo_http_response *uo_http_response_create(void);

bool uo_http_response_parse_status_line(
    uo_http_response *,
    uo_buf);

void uo_http_response_destroy(
    uo_http_response *);

#ifdef __cplusplus
}
#endif

#endif
