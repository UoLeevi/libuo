#include "uo_http_header.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

const char *uo_http_header_content_type_for_path(
    const char *path)
{
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) 
        return "";

    switch (tolower(dot[1]))
    {
        case 't': return UO_HTTP_HEADER_CONTENT_TYPE_TEXT;
        case 'h': return UO_HTTP_HEADER_CONTENT_TYPE_HTML;
        case 'c': return UO_HTTP_HEADER_CONTENT_TYPE_CSS;
        case 'j': return UO_HTTP_HEADER_CONTENT_TYPE_JS;
        default: return "";
    }
}