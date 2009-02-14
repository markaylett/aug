/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

/**
   Returns #AUG_FAILNONE if connection is not established.
*/

AUGNET_API aug_result
aug_established(aug_sd sd);

#endif /* AUGNET_INET_H */
