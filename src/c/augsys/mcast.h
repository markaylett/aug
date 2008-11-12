/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
 * header size is 60 bytes.  It should, therefore, be reasonable safe to
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
