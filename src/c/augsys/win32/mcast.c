/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"

#include "iptypes_.h"
#include <iphlpapi.h>

#if 0
static int
findif_(int af, unsigned int iface,
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
        for (i = 0; it && i < iface; it = it->Next, ++i)
            ;

        if (it)
            i = fn(arg, iface, it);
        else {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                           AUG_MSG("interface '%d' does not exist"), iface);
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
sockaddr_(void* arg, unsigned int iface, PIP_ADAPTER_ADDRESSES adapter)
{
    struct sockaddr_in* out = arg;
    PIP_ADAPTER_UNICAST_ADDRESS it = adapter->FirstUnicastAddress;

    /* Iteration is not strictly necessary: the addresses should already be
       filtered by AF_INET. */

    for (; it ; it = it->Next)
        if (AF_INET == it->Address.lpSockaddr->sa_family) {
            memcpy(out, it->Address.lpSockaddr, sizeof(*out));
            return 0;
        }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no address for interface '%d'"), iface);
    return -1;
}

static int
index_(void* arg, unsigned int iface, PIP_ADAPTER_ADDRESSES adapter)
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
                   AUG_MSG("no address for interface '%d'"), iface);
    return -1;
}

static int
getsockaddr_(struct sockaddr_in* addr, unsigned int iface)
{
    return findif_(AF_INET, iface, sockaddr_, &addr);
}

static int
getindex_(DWORD* index, unsigned int iface)
{
    return findif_(AF_INET6, iface, index_, &index);
}
#endif

AUGSYS_API int
aug_joinmcast(int s, const struct aug_ipaddr* addr, unsigned int iface)
{
#if 0
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = ((struct sockaddr_in*)addr)->sin_addr.s_addr;

    if (0 == iface) {
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    } else {

        struct struct sockaddr_in iaddr;
        if (-1 == getsockaddr(&iaddr, iface))
            return -1;

        memcpy(&mreq.imr_interface,
               ((struct sockaddr_in*)&ifreq.ifr_addr).sin_addr,
               sizeof(struct in_addr));
    }

    return aug_setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
                          sizeof(mreq));
#endif
    return 0;
}

AUGSYS_API int
aug_leavemcast(int s, const struct aug_ipaddr* addr, unsigned int iface)
{
    return 0;
}

AUGSYS_API int
aug_setmcastif(int s, unsigned int iface)
{
    return 0;
}

AUGSYS_API int
aug_setmcastloop(int s, int on)
{
    return 0;
}

AUGSYS_API int
aug_setmcastttl(int s, int ttl)
{
    return 0;
}
