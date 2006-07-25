/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_INET_H
#define AUGNET_INET_H

#include "augnet/config.h"

#include "augsys/socket.h"

/* AUG_MAXSOCKADDR should be large enough to accomodate hostnames: on Linux,
   MAXHOSTNAMELEN is defined as 64. */

struct aug_hostserv {
    char* host_, * serv_;
    char data_[AUG_MAXADDRLEN];
};

/* Implementations of the original, classic functions by Richard Stevens. */

AUGNET_API int
aug_tcpconnect(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API int
aug_tcplisten(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API int
aug_udpclient(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API int
aug_udpconnect(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API int
aug_udpserver(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API struct aug_hostserv*
aug_parsehostserv(const char* src, struct aug_hostserv* dst);

AUGNET_API int
aug_setnodelay(int fd, int on);

#endif /* AUGNET_INET_H */
