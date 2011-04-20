/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

    /* SIGCALL: ioctl: */
    if (ioctl(sd, SIOCGIFADDR, &ifreq) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    addr->s_addr = ((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr.s_addr;
    return 0;
}

static aug_result
getifindex_(unsigned* index, const char* ifname)
{
    if (!(*index = if_nametoindex(ifname))) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, ENODEV);
        return -1;
    }
    return 0;
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

        if (ifname) {
            if (getifaddr_(sd, &un.ipv4_.imr_interface, ifname) < 0)
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {
            if (getifindex_(&un.ipv6_.ipv6mr_interface, ifname) < 0)
                return -1;
        } else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
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

        if (ifname) {
            if (getifaddr_(sd, &un.ipv4_.imr_interface, ifname) < 0)
                return -1;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {
            if (getifindex_(&un.ipv6_.ipv6mr_interface, ifname) < 0)
                return -1;
        } else
            un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
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

    if ((af = aug_getfamily(sd)) < 0)
        return -1;

    switch (af) {
    case AF_INET:

        if (getifaddr_(sd, &un.ipv4_, ifname) < 0)
            return -1;
        return aug_setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, &un.ipv4_,
                              sizeof(un.ipv4_));

#if HAVE_IPV6
    case AF_INET6:

        if (getifindex_(&un.ipv6_, ifname) < 0)
            return -1;
        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}

AUGSYS_API aug_result
aug_setmcastloop(aug_sd sd, aug_bool on)
{
    aug_result af;
    union {
        u_char ipv4_;
#if HAVE_IPV6
        u_int ipv6_;
#endif /* HAVE_IPV6 */
    } un;

    if ((af = aug_getfamily(sd)) < 0)
        return -1;

    switch (af) {
    case AF_INET:
        un.ipv4_ = on ? 1 : 0;
        return aug_setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &un.ipv4_,
                              sizeof(un.ipv4_));
#if HAVE_IPV6
    case AF_INET6:
        un.ipv6_ = on ? 1 : 0;
        return aug_setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                              &un.ipv6_, sizeof(un.ipv6_));
#endif /* HAVE_IPV6 */
    }

    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
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

    if ((af = aug_getfamily(sd)) < 0)
        return -1;

    switch (af) {
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

    aug_setposixerror(aug_tlx, __FILE__, __LINE__, EAFNOSUPPORT);
    return -1;
}
