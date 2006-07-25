/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/socket.h"

static const char rcsid[] = "$Id:$";

#if !defined(_WIN32)
# define SETAFNOSUPPORT_() \
    aug_setposixerrinfo(__FILE__, __LINE__, EAFNOSUPPORT)
# include "augsys/posix/socket.c"
#else /* _WIN32 */
# define SETAFNOSUPPORT_() \
    aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT)
# include "augsys/win32/socket.c"
#endif /* _WIN32 */

typedef struct aug_inetaddr ipv4first_;

static const ipv4first_ ipv4any_ = {
    AF_INET,
    { { { { 0, 0, 0, 0 } } } }
};

static const ipv4first_ ipv4loopback_ = {
    AF_INET,
    { { { { 127, 0, 0, 1 } } } }
};

typedef struct {
    short family_;
    union {
        struct in6_addr ipv6_;
        struct in_addr ipv4_;
    } un_;
} ipv6first_;

static const ipv6first_ ipv6any_ = {
    AF_INET6,
    { { { IN6ADDR_ANY_INIT } } }
};

static const ipv6first_ ipv6loopback_ = {
    AF_INET6,
    { { { IN6ADDR_LOOPBACK_INIT }  } }
};

AUGSYS_API int
aug_setreuseaddr(int s, int on)
{
    return aug_setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

AUGSYS_API int
aug_getfamily(int s)
{
    struct aug_endpoint ep;
    if (!aug_getsockname(s, &ep))
        return -1;

    return ep.un_.family_;
}

AUGSYS_API struct aug_endpoint*
aug_setinetaddr(struct aug_endpoint* ep, const struct aug_inetaddr* addr)
{
    switch (ep->un_.family_ = addr->family_) {
    case AF_INET:
        ep->len_ = sizeof(ep->un_.ipv4_);
        ep->un_.ipv4_.sin_addr.s_addr = addr->un_.ipv4_.s_addr;
        break;
    case AF_INET6:
        ep->len_ = sizeof(ep->un_.ipv6_);
        memcpy(&ep->un_.ipv6_.sin6_addr, &addr->un_.ipv6_,
               sizeof(addr->un_.ipv6_));
        break;
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return ep;
}

AUGSYS_API struct aug_inetaddr*
aug_getinetaddr(const struct aug_endpoint* ep, struct aug_inetaddr* addr)
{
    switch (addr->family_ = ep->un_.family_) {
    case AF_INET:
        addr->un_.ipv4_.s_addr = ep->un_.ipv4_.sin_addr.s_addr;
        break;
    case AF_INET6:
        memcpy(&addr->un_, &ep->un_.ipv6_.sin6_addr,
               sizeof(ep->un_.ipv6_.sin6_addr));
        break;
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API const struct aug_inetaddr*
aug_inetany(int af)
{
    const void* addr;
    switch (af) {
    case AF_INET:
        addr = &ipv4any_;
        break;
    case AF_INET6:
        addr = &ipv6any_;
        break;
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API const struct aug_inetaddr*
aug_inetloopback(int af)
{
    const void* addr;
    switch (af) {
    case AF_INET:
        addr = &ipv4loopback_;
        break;
    case AF_INET6:
        addr = &ipv6loopback_;
        break;
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}
