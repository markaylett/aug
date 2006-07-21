/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"

AUGSYS_API int
aug_joinmcast(int s, const struct aug_ipaddr* addr, unsigned int iface)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;
    mreq.imr_interface.s_addr = iface;
    return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
                          sizeof(mreq));
}

AUGSYS_API int
aug_leavemcast(int s, const struct aug_ipaddr* addr, unsigned int iface)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;;
    mreq.imr_interface.s_addr = iface;
    return aug_setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq,
                          sizeof(mreq));
}

AUGSYS_API int
aug_setmcastif(int s, unsigned int iface)
{
    return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, NULL, 0);
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
