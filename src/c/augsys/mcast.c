/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# include <netinet/in.h>
#else /* _WIN32 */
# include <ws2tcpip.h>
#endif /* _WIN32 */

AUGSYS_API int
aug_addmcastmem(int s, unsigned long mcast, unsigned long iface)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = mcast;
    mreq.imr_interface.s_addr = iface;
    return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
                          sizeof(mreq));
}

AUGSYS_API int
aug_dropmcastmem(int s, unsigned long mcast, unsigned long iface)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = mcast;
    mreq.imr_interface.s_addr = iface;
    return aug_setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq,
                          sizeof(mreq));
}

AUGSYS_API int
aug_setmcastif(int s, unsigned long addr)
{
    return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &addr,
                          sizeof(addr));
}

AUGSYS_API int
aug_setmcastloop(int s, int on)
{
    return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &on, sizeof(on));
}

AUGSYS_API int
aug_setmcastttl(int s, int ttl)
{
    return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
                          sizeof(ttl));
}
