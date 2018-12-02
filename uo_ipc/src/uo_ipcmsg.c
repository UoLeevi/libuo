#include "uo_ipcmsg.h"
#include "uo_io.h"
#include "uo_err.h"

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

/*  uo_ipcmsg internal bit structure

         |0 1 2 3 4 5 6 7 8 9 A B C D E F|0 1 2 3 4 5 6 7 8 9 A B C D E F |
    -----------------------------------------------------------------------
    0x00 |       p a y l o a d   l e n g t h             (32 bits)        |
    -----------------------------------------------------------------------
    0x20 |       p a y l o a d   (null terminated for ease of use)        |
     ... |                                                                |
    ----------------------------------------------------------------------- */

uint32_t uo_ipcmsg_get_payload_len(
    uo_ipcmsg ipcmsg)
{
    return ntohl(*(uint32_t *)ipcmsg);
}

void uo_ipcmsg_set_payload_len(
    uo_ipcmsg ipcmsg,
    uint32_t payload_len)
{
    *(uint32_t *)ipcmsg = htonl(payload_len);
}

char *uo_ipcmsg_get_payload(
    uo_ipcmsg ipcmsg)
{
    return ipcmsg + sizeof(uint32_t);
}

bool uo_ipcmsg_write(
    int wfd,
    const char *src,
    uint32_t len)
{
    char buf[0x100];

    uo_ipcmsg_set_payload_len(buf, len);
    char *p = uo_ipcmsg_get_payload(buf);

    bool is_long_msg = len > sizeof buf - (p - buf);

    memcpy(p, src, is_long_msg ? sizeof buf - (p - buf) : len);

    ssize_t wlen = uo_io_write(wfd, buf, is_long_msg ? sizeof buf : sizeof(uint32_t) + len);
    if (wlen <= 0)
        uo_err_return(false, "Error while sending ipcmsg.");

    len -= wlen - sizeof(uint32_t);
    src += wlen - sizeof(uint32_t);

    while (len)
    {
        wlen = uo_io_write(wfd, src, len);
        if (wlen <= 0)
            uo_err_return(false, "Error while sending ipcmsg.");
        
        len -= wlen;
        src += wlen;
    }

    return true;
}
