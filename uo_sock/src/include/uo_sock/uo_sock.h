#ifndef UO_SOCK_H
#define UO_SOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined WIN32 || _WIN32
#define UO_SOCK_WIN32
#else
#define UO_SOCK_POSIX
#endif

#ifdef UO_SOCK_WIN32 
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef UO_SOCK_POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <stdbool.h>

bool uo_sock_init(void);

int uo_setsockopt(
    int socket,
    int level,
    int option_name,
    const void *option_value,
    socklen_t option_len);

#ifdef __cplusplus
}
#endif

#endif