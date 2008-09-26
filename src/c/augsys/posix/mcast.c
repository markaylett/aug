/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mcast.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"

#include <net/if.h>
#include <sys/ioctl.h>

#if HAVE_SYS_SOCKIO_H
# include <sys/sockio.h> /* SIOCGIFADDR */
#endif /* HAVE_SYS_SOCKIO_H */

static aug_result
getifaddr_(aug_sd sd, struct in_addr* addr, const char* ifname)
{
    struct ifreq ifreq;
    aug_strlcpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name));

    if (-1 == ioctl(sd, SIOCGIFADDR, &ifreq))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    addr->s_addr = ((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr.s_addr;
    return AUG_SUCCESS;
}

static aug_result
getifindex_(unsigned* index, const char* ifname)
{
    if (!(*index = if_nametoindex(ifname)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, ENODEV);
    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_joinmcast(aug_sd sd, const struct aug_inetaddr* addr, const char* ifname)
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

        if (ifname)
            aug_verify(getifaddr_(sd, &un.ipv4_.imr_interface, ifname));
        else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname)
            aug_verify(getifindex_(&un.ipv6_.ipv6mr_interface, ifname));
        else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
}

AUGSYS_API aug_result
aug_leavemcast(aug_sd sd, const struct aug_inetaddr* addr, const char* ifname)
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

        if (ifname)
            aug_verify(getifaddr_(sd, &un.ipv4_.imr_interface, ifname));
        else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname)
            aug_verify(getifindex_(&un.ipv6_.ipv6mr_interface, ifname));
        else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
}

AUGSYS_API aug_result
aug_setmcastif(aug_sd sd, const char* ifname)
{
    aug_result af;
    union {
        struct in_addr ipv4_;
#if HAVE_IPV6
        u_int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (AUG_ISFAIL(af = aug_getfamily(sd)))
        return af;

    switch (AUG_RESULT(af)) {
    case AF_INET:

        aug_verify(getifaddr_(sd, &un.ipv4_, ifname));
        return aug_setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

        aug_verify(getifindex_(&un.ipv6_, ifname));
        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
}

AUGSYS_API aug_result
aug_setmcastloop(aug_sd sd, int on)
{
    aug_result af;
    union {
        u_char ipv4_;
#if HAVE_IPV6
        u_int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (AUG_ISFAIL(af = aug_getfamily(sd)))
        return af;

    switch (AUG_RESULT(af)) {
    case AF_INET:
        un.ipv4_ = on;
        return aug_setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &un.ipv4_,
                              sizeof(un.ipv4_));
#if HAVE_IPV6
    case AF_INET6:
        un.ipv6_ = on;
        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
}

AUGSYS_API aug_result
aug_setmcastttl(aug_sd sd, int ttl)
{
    aug_rint af;
    union {
        u_char ipv4_;
#if HAVE_IPV6
        int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if (AUG_ISFAIL(af = aug_getfamily(sd)))
        return af;

    switch (AUG_RESULT(af)) {
    case AF_INET:
        un.ipv4_ = ttl;
        return aug_setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, &un.ipv4_,
                              sizeof(un.ipv4_));
#if HAVE_IPV6
    case AF_INET6:
        un.ipv6_ = ttl;
        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
}
