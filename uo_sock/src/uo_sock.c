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

#ifdef UO_SOCK_WIN32
    is_init &= WSAStartup(MAKEWORD(1, 1), &WsaDat) == 0;
#endif

    atexit(uo_sock_quit);
    return is_init;
}

int uo_setsockopt(
    int socket,
    int level,
    int option_name,
    const void *option_value,
    socklen_t option_len)
{
#ifdef UO_SOCK_WIN32
    switch (option_name)
    {
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
        {
            const struct timeval *tv = option_value;
            DWORD opt = tv->tv_sec * 1000 + tv->tv_usec / 1000;
            return setsockopt(socket, level, option_name, (char *)&opt, sizeof opt);
        }
    }
#endif
    return setsockopt(socket, level, option_name, option_value, option_len);
}