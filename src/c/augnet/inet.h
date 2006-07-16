/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_INET_H
#define AUGNET_INET_H

#include "augnet/config.h"

#include "augsys/socket.h"

struct aug_sockaddr {
    socklen_t addrlen_;
    union {
        struct sockaddr sa_;
        struct sockaddr_in sin_;
        struct sockaddr_in6 sin6_;
        char data_[AUG_MAXSOCKADDR];
    } un_;
};

/* Implementations of the original, classic functions by Richard Stevens. */

AUGNET_API int
aug_tcpconnect(const char* host, const char* serv, struct aug_sockaddr* addr);

AUGNET_API int
aug_tcplisten(const char* host, const char* serv, struct aug_sockaddr* addr);

AUGNET_API int
aug_udpclient(const char* host, const char* serv, struct aug_sockaddr* addr);

AUGNET_API int
aug_udpconnect(const char* host, const char* serv, struct aug_sockaddr* addr);

AUGNET_API int
aug_udpserver(const char* host, const char* serv, struct aug_sockaddr* addr);

AUGNET_API struct sockaddr*
aug_parseinet(struct aug_sockaddr* dst, const char* src);

AUGNET_API int
aug_setnodelay(int fd, int on);

#endif /* AUGNET_INET_H */
