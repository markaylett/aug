/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"

#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>

static int
getifaddr_(int s, struct in_addr* addr, unsigned int ifindex)
{
    struct ifreq ifreq;
    if (!if_indextoname(ifindex, ifreq.ifr_name)) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENXIO);
        return -1;
    }

    if (-1 == ioctl(s, SIOCGIFADDR, &ifreq)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    addr->s_addr = ((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr.s_addr;
    return 0;
}

AUGSYS_API int
aug_joinmcast(int s, const struct aug_inetaddr* addr, unsigned int ifindex)
{
    union {
        struct ip_mreq ipv4_;
        struct ipv6_mreq ipv6_;
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifindex) {

            if (-1 == getifaddr_(s, &un.ipv4_.imr_interface, ifindex))
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        un.ipv6_.ipv6mr_interface = ifindex;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_leavemcast(int s, const struct aug_inetaddr* addr, unsigned int ifindex)
{
    union {
        struct ip_mreq ipv4_;
        struct ipv6_mreq ipv6_;
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifindex) {

            if (-1 == getifaddr_(s, &un.ipv4_.imr_interface, ifindex))
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        un.ipv6_.ipv6mr_interface = ifindex;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastif(int s, unsigned int ifindex)
{
    int af;
    union {
        struct in_addr ipv4_;
        u_int ipv6_;
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:

        if (-1 == getifaddr_(s, &un.ipv4_, ifindex))
            return -1;

        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

        un.ipv6_ = ifindex;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastloop(int s, int on)
{
    int af;
    union {
        u_char ipv4_;
        u_int ipv6_;
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:
        un.ipv4_ = on;
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:
        un.ipv6_ = on;
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcasthops(int s, int hops)
{
    int af;
    union {
        u_char ipv4_;
        int ipv6_;
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:
        un.ipv4_ = hops;
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:
        un.ipv6_ = hops;
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}
