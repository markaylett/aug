/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MCAST_H
#define AUGSYS_MCAST_H

#include "augsys/socket.h"

AUGSYS_API int
aug_joinmcast(int s, const struct aug_ipaddr* addr, unsigned int ifindex);

AUGSYS_API int
aug_leavemcast(int s, const struct aug_ipaddr* addr, unsigned int ifindex);

AUGSYS_API int
aug_setmcastif(int s, unsigned int ifindex);

AUGSYS_API int
aug_setmcastloop(int s, int on);

AUGSYS_API int
aug_setmcasthops(int s, int hops);

#endif /* AUGSYS_MCAST_H */
