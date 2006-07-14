/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MCAST_H
#define AUGSYS_MCAST_H

#include "augsys/socket.h"

AUGSYS_API int
aug_addmcastmem(int s, unsigned long mcast, unsigned long iface);

AUGSYS_API int
aug_dropmcastmem(int s, unsigned long mcast, unsigned long iface);

AUGSYS_API int
aug_setmcastif(int s, unsigned long addr);

AUGSYS_API int
aug_setmcastloop(int s, int on);

AUGSYS_API int
aug_setmcastttl(int s, int ttl);

#endif /* AUGSYS_MCAST_H */
