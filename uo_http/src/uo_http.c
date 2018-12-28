#include "uo_http.h"
#include "uo_tcp.h"

static bool is_init;

bool uo_http_init(void)
{
    if (is_init)
        return is_init;

    is_init = true;

    is_init &= uo_tcp_init();

    return is_init;
}
