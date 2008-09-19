/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/socket.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errinfo.h"
#include "augctx/errno.h"

#if !defined(_WIN32)
# define SETAFNOSUPPORT_() \
    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT)
# include "augsys/posix/socket.c"
#else /* _WIN32 */
# define SETAFNOSUPPORT_() \
    aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, WSAEAFNOSUPPORT)
# include "augsys/win32/socket.c"
# define snprintf _snprintf
#endif /* _WIN32 */

#include <assert.h>
#include <stdio.h>

struct ipv4_ {
    short family_;
    union {
        unsigned char data_[4];
        struct in_addr ipv4_;
#if HAVE_IPV6
        struct sockaddr_in6 ipv6_;
#endif /* HAVE_IPV6 */
        char pad_[16];
    } un_;
};

static const struct ipv4_ ipv4any_ = {
    AF_INET,
    { { 0, 0, 0, 0 } }
};

static const struct ipv4_ ipv4loopback_ = {
    AF_INET,
    { { 127, 0, 0, 1 } }
};

#if HAVE_IPV6
struct ipv6_ {
    short family_;
    union {
        unsigned char data_[16];
        struct in_addr ipv4_;
        struct in6_addr ipv6_;
        char pad_[16];
    } un_;
};

static const struct ipv6_ ipv6any_ = {
    AF_INET6,
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

static const struct ipv6_ ipv6loopback_ = {
    AF_INET6,
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } }
};
#endif /* HAVE_IPV6 */

AUGSYS_API char*
aug_endpointntop(const struct aug_endpoint* src, char* dst, socklen_t len)
{
    struct aug_inetaddr addr;
    char host[AUG_MAXHOSTNAMELEN + 1];
    const char* fmt;
    int ret;

    assert(src && dst && len);

    /* Get the hostname. */

    if (!aug_getinetaddr(src, &addr)
        || !aug_inetntop(&addr, host, sizeof(host)))
        return NULL;

    /* Select format based on family. */

    switch (src->un_.family_) {
    case AF_INET:
        fmt = "%s:%d";
        break;
#if HAVE_IPV6
    case AF_INET6:
        fmt = "[%s]:%d";
        break;
#endif /* HAVE_IPV6 */
    default:
#if !defined(_WIN32)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, EAFNOSUPPORT);
#else /* _WIN32 */
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, WSAEAFNOSUPPORT);
#endif /* _WIN32 */
        return NULL;
    }

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = snprintf(dst, len, fmt, host, (int)ntohs(src->un_.all_.port_));
    AUG_SNTRUNCF(dst, len, ret);

    if (ret < 0) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    }

    return dst;
}

AUGSYS_API aug_result
aug_setreuseaddr(aug_sd sd, int on)
{
    return aug_setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

AUGSYS_API struct aug_endpoint*
aug_getendpoint(const struct addrinfo* addr, struct aug_endpoint* ep)
{
    ep->len_ = (socklen_t)addr->ai_addrlen;
    memcpy(&ep->un_, addr->ai_addr, addr->ai_addrlen);
    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_setinetaddr(struct aug_endpoint* ep, const struct aug_inetaddr* addr)
{
    switch (ep->un_.family_ = addr->family_) {
    case AF_INET:
        ep->len_ = sizeof(ep->un_.ipv4_);
        ep->un_.ipv4_.sin_addr.s_addr = addr->un_.ipv4_.s_addr;
        break;
#if HAVE_IPV6
    case AF_INET6:
        ep->len_ = sizeof(ep->un_.ipv6_);
        memcpy(&ep->un_.ipv6_.sin6_addr, &addr->un_.ipv6_,
               sizeof(addr->un_.ipv6_));
        break;
#endif /* HAVE_IPV6 */
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
#if HAVE_IPV6
    case AF_INET6:
        memcpy(&addr->un_, &ep->un_.ipv6_.sin6_addr,
               sizeof(ep->un_.ipv6_.sin6_addr));
        break;
#endif /* HAVE_IPV6 */
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
#if HAVE_IPV6
    case AF_INET6:
        addr = &ipv6any_;
        break;
#endif /* HAVE_IPV6 */
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
#if HAVE_IPV6
    case AF_INET6:
        addr = &ipv6loopback_;
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }
    return addr;
}

AUGSYS_API aug_bool
aug_acceptlost(struct aug_errinfo* errinfo)
{
    /* FIXME: would be better if aug_tlerr were used. */

    switch (aug_errno(errinfo)) {
    case ECONNABORTED:
#if defined(EPROTO)
    case EPROTO:
#endif /* EPROTO */
    case EWOULDBLOCK:
        return AUG_TRUE;
    }

    return AUG_FALSE;
}

AUGSYS_API aug_result
aug_setsockerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                   aug_sd sd)
{
    int err = 0;
    socklen_t len = sizeof(err);

    if (AUG_ISFAIL(aug_getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len))) {

        /* Maintain semantics with existing errinfo functions. */

        const struct aug_errinfo* src = aug_tlerr;
        aug_seterrinfo(errinfo, src->file_, src->line_, src->src_, src->num_,
                       src->desc_);
        return AUG_FAILERROR;
    }

    return aug_setposixerrinfo(errinfo, file, line, err);
}
