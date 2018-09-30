#include "uo_sock.h"

#include <stdlib.h>

static bool is_init;

#ifdef UO_SOCK_WIN32
static WSADATA WsaDat;
#endif

static void uo_sock_quit()
{

}

bool uo_sock_init()
{
    if (is_init)
        return true;

    is_init = true;

    bool s = true;

#ifdef UO_SOCK_WIN32
    s = WSAStartup(MAKEWORD(1, 1), &WsaDat) != 0;
#endif

    atexit(uo_sock_quit);
    return s;
}