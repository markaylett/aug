/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/string.h"

#include <net/if.h>
#include <sys/ioctl.h>

#if HAVE_SYS_SOCKIO_H
# include <sys/sockio.h> /* SIOCGIFADDR */
#endif /* HAVE_SYS_SOCKIO_H */

static int
getifaddr_(int s, struct in_addr* addr, const char* ifname)
{
    struct ifreq ifreq;
    aug_strlcpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));

    if (-1 == ioctl(s, SIOCGIFADDR, &ifreq)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    addr->s_addr = ((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr.s_addr;
    return 0;
}

static int
getifindex_(unsigned* index, const char* ifname)
{
    if (!(*index = if_nametoindex(ifname))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENODEV);
        return -1;
    }
    return 0;
}

AUGSYS_API int
aug_joinmcast(int s, const struct aug_inetaddr* addr, const char* ifname)
{
    union {
        struct ip_mreq ipv4_;
#if HAVE_IPV6
        struct ipv6_mreq ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifname) {

            if (-1 == getifaddr_(s, &un.ipv4_.imr_interface, ifname))
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {
            if (-1 == getifindex_(&un.ipv6_.ipv6mr_interface, ifname))
                return -1;
        } else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_leavemcast(int s, const struct aug_inetaddr* addr, const char* ifname)
{
    union {
        struct ip_mreq ipv4_;
#if HAVE_IPV6
        struct ipv6_mreq ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifname) {

            if (-1 == getifaddr_(s, &un.ipv4_.imr_interface, ifname))
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {
            if (-1 == getifindex_(&un.ipv6_.ipv6mr_interface, ifname))
                return -1;
        } else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastif(int s, const char* ifname)
{
    int af;
    union {
        struct in_addr ipv4_;
#if HAVE_IPV6
        u_int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:

        if (-1 == getifaddr_(s, &un.ipv4_, ifname))
            return -1;

        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

        if (-1 == getifindex_(&un.ipv6_, ifname))
            return -1;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastloop(int s, int on)
{
    int af;
    union {
        u_char ipv4_;
#if HAVE_IPV6
        u_int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:
        un.ipv4_ = on;
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &un.ipv4_,
                              sizeof(un.ipv4_));
#if HAVE_IPV6
    case AF_INET6:
        un.ipv6_ = on;
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastttl(int s, int ttl)
{
    int af;
    union {
        u_char ipv4_;
#if HAVE_IPV6
        int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:
        un.ipv4_ = ttl;
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &un.ipv4_,
                              sizeof(un.ipv4_));
#if HAVE_IPV6
    case AF_INET6:
        un.ipv6_ = ttl;
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}
