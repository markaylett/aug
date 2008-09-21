/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_INET_H
#define AUGNET_INET_H

/**
 * @file augnet/inet.h
 *
 * TCP functions.
 *
 * Implementations of the original, classic functions by Richard Stevens.
 *
 * @todo introduce timeout for aug_tcpclient() by implementing in terms of
 * aug_tryconnect().
 */

#include "augnet/config.h"

#include "augsys/socket.h"

/**
 * AUG_MAXSOCKADDR should be large enough to accomodate hostnames: on Linux,
 * MAXHOSTNAMELEN is defined as 64.
 */

struct aug_hostserv {
    char* host_, * serv_;
    char data_[AUG_MAXHOSTSERVLEN + 1];
};

AUGNET_API aug_sd
aug_tcpclient(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API aug_sd
aug_tcpserver(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API aug_sd
aug_udpclient(const char* host, const char* serv, struct aug_endpoint* ep,
              aug_bool connect);

AUGNET_API aug_sd
aug_udpserver(const char* host, const char* serv, struct aug_endpoint* ep);

AUGNET_API struct aug_hostserv*
aug_parsehostserv(const char* src, struct aug_hostserv* dst);

AUGNET_API aug_result
aug_setnodelay(aug_sd sd, int on);

AUGNET_API aug_result
aug_established(aug_sd sd);

#endif /* AUGNET_INET_H */
