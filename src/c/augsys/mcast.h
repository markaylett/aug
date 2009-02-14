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
#ifndef AUGSYS_MCAST_H
#define AUGSYS_MCAST_H

/**
 * @file augsys/mcast.h
 *
 * Multicast sockets.
 */

#include "augsys/socket.h"

/**
 * The minimum size of an IP datagram is 576 bytes.  The maximum IP datagram
 * header size is 60 bytes.  It should, therefore, be reasonably safe to
 * assume that 516 bytes can be sent in a datagram's data section without
 * fragmentation.
 */

#if !defined(AUG_PACKETSIZE)
# define AUG_PACKETSIZE 512
#endif /* !AUG_PACKETSIZE */

AUGSYS_API aug_result
aug_joinmcast(aug_sd sd, const struct aug_inetaddr* addr, const char* ifname);

AUGSYS_API aug_result
aug_leavemcast(aug_sd sd, const struct aug_inetaddr* addr,
               const char* ifname);

AUGSYS_API aug_result
aug_setmcastif(aug_sd sd, const char* ifname);

/**
 * True if multicast traffic should be looped-back to originating host.
 */

AUGSYS_API aug_result
aug_setmcastloop(aug_sd sd, aug_bool on);

AUGSYS_API aug_result
aug_setmcastttl(aug_sd sd, int ttl);

#endif /* AUGSYS_MCAST_H */
