/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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

AUGSYS_API int
aug_joinmcast(int s, const struct aug_inetaddr* addr, const char* ifname);

AUGSYS_API int
aug_leavemcast(int s, const struct aug_inetaddr* addr, const char* ifname);

AUGSYS_API int
aug_setmcastif(int s, const char* ifname);

AUGSYS_API int
aug_setmcastloop(int s, int on);

AUGSYS_API int
aug_setmcastttl(int s, int ttl);

#endif /* AUGSYS_MCAST_H */
