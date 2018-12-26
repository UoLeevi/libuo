#include "uo_tcp.h"
#include "uo_cb.h"
#include "uo_io.h"
#include "uo_sock.h"

static bool is_init;

bool uo_tcp_init(void)
{
    if (is_init)
        return is_init;

    is_init = true;

    is_init &= uo_cb_init();
    is_init &= uo_sock_init();
    is_init &= uo_io_init();

    return is_init;
}
