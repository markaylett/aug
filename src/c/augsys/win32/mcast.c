/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"

#if defined(__GNUC__)
# include "iptypes_.h"
#endif /* __GNUC__ */
#include <iphlpapi.h>

static int
findif_(int af, unsigned int ifindex,
        int (*fn)(void*, unsigned int, PIP_ADAPTER_ADDRESSES), void* arg)
{
    PIP_ADAPTER_ADDRESSES list = NULL;
    ULONG len = 0, ret = 0;
    int i;

    /* The size of the buffer can be different between consecutive API calls.
       In most cases, i < 2 is sufficient; one call to get the size and one
       call to get the actual parameters.  But if one more interface is added
       or addresses are added, the call again fails with BUFFER_OVERFLOW.  So
       the number is picked slightly greater than 2. */

    for (i = 0; i < 5; i++) {

        ret = GetAdaptersAddresses(af, 0, NULL, list, &len);
        if (ERROR_BUFFER_OVERFLOW != ret)
            break;

        if (list)
            free(list);

        list = malloc(len);
        if (!list) {
            ret = GetLastError();
            break;
        }
    }

    if (NO_ERROR == ret) {

        PIP_ADAPTER_ADDRESSES it = list;
        for (i = 0; it && i < (int)ifindex; it = it->Next, ++i)
            ;

        if (it)
            i = fn(arg, ifindex, it);
        else {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                           AUG_MSG("interface '%d' does not exist"), ifindex);
            i = -1;
        }

    } else {
        aug_setwin32errinfo(__FILE__, __LINE__, ret);
        i = -1;
    }

    if (list)
        free(list);
    return i;
}

static int
ifaddr_(void* arg, unsigned int ifindex, PIP_ADAPTER_ADDRESSES adapter)
{
    struct in_addr* out = arg;
    PIP_ADAPTER_UNICAST_ADDRESS it = adapter->FirstUnicastAddress;

    /* Iteration is not strictly necessary: the addresses should already be
       filtered by AF_INET. */

    for (; it ; it = it->Next)
        if (AF_INET == it->Address.lpSockaddr->sa_family) {
            out->s_addr = ((struct sockaddr_in*)it->Address.lpSockaddr)
                ->sin_addr.s_addr;
            return 0;
        }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no address for interface '%d'"), ifindex);
    return -1;
}

static int
ifindex_(void* arg, unsigned int ifindex, PIP_ADAPTER_ADDRESSES adapter)
{
    DWORD* out = arg;
    PIP_ADAPTER_UNICAST_ADDRESS it = adapter->FirstUnicastAddress;

    /* Iteration is not strictly necessary: the addresses should already be
       filtered by AF_INET6. */

    for (; it ; it = it->Next)
        if (AF_INET6 == it->Address.lpSockaddr->sa_family) {
            *out = adapter->Ipv6IfIndex;
            return 0;
        }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no address for interface '%d'"), ifindex);
    return -1;
}

static int
getifaddr_(struct in_addr* addr, const char* ifname)
{
    unsigned int ifindex = atoi(ifname);
    return findif_(AF_INET, ifindex, ifaddr_, &addr);
}

static int
getifindex_(DWORD* index, const char* ifname)
{
    unsigned int ifindex = atoi(ifname);
    return findif_(AF_INET6, ifindex, ifindex_, &index);
}

AUGSYS_API int
aug_joinmcast(int s, const struct aug_inetaddr* addr, const char* ifname)
{
    union {
        struct ip_mreq ipv4_;
        struct ipv6_mreq ipv6_;
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifname) {

            struct in_addr ifaddr;
            if (-1 == getifaddr_(&ifaddr, ifname))
                return -1;

            un.ipv4_.imr_interface.s_addr = ifaddr.s_addr;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {

            DWORD i;
            if (-1 == getifindex_(&i, ifname))
                return -1;

			un.ipv6_.ipv6mr_interface = i;
        } else
			un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_leavemcast(int s, const struct aug_inetaddr* addr, const char* ifname)
{
    union {
        struct ip_mreq ipv4_;
        struct ipv6_mreq ipv6_;
    } un;

    switch (addr->family_) {
    case AF_INET:

        un.ipv4_.imr_multiaddr.s_addr = addr->un_.ipv4_.s_addr;

        if (ifname) {

            struct in_addr ifaddr;
            if (-1 == getifaddr_(&ifaddr, ifname))
                return -1;

            un.ipv4_.imr_interface.s_addr = ifaddr.s_addr;
        } else
            un.ipv4_.imr_interface.s_addr = htonl(INADDR_ANY);

        return aug_setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

		memcpy(&un.ipv6_.ipv6mr_multiaddr, &addr->un_.ipv6_,
			   sizeof(addr->un_.ipv6_));

        if (ifname) {

            DWORD i;
            if (-1 == getifindex_(&i, ifname))
                return -1;

			un.ipv6_.ipv6mr_interface = i;
        } else
			un.ipv6_.ipv6mr_interface = 0;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastif(int s, const char* ifname)
{
    int af;
    union {
        struct in_addr ipv4_;
        DWORD ipv6_;
    } un;

    if (-1 == (af = aug_getfamily(s)))
        return -1;

    switch (af) {
    case AF_INET:

        if (-1 == getifaddr_(&un.ipv4_, ifname))
            return -1;

        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &un.ipv4_,
                              sizeof(un.ipv4_));

    case AF_INET6:

        if (-1 == getifindex_(&un.ipv6_, ifname))
            return -1;

        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                              &un.ipv6_, sizeof(un.ipv6_));
    }

    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcastloop(int s, int on)
{
    int af = aug_getfamily(s);
    if (-1 == af)
        return -1;

    /* On Windows, both the IPV4 and IPV6 options expect a DWORD. */

    switch (af) {
    case AF_INET:
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &on,
                              sizeof(on));

    case AF_INET6:
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                              &on, sizeof(on));
    }

    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
    return -1;
}

AUGSYS_API int
aug_setmcasthops(int s, int hops)
{
    int af = aug_getfamily(s);
    if (-1 == af)
        return -1;

    /* On Windows, both the IPV4 and IPV6 options expect a DWORD. */

    switch (af) {
    case AF_INET:
        return aug_setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &hops,
                              sizeof(hops));

    case AF_INET6:
        return aug_setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                              &hops, sizeof(hops));
    }

    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
    return -1;
}
