/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MCAST_H
#define AUGSYS_MCAST_H

#include "augsys/socket.h"

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
