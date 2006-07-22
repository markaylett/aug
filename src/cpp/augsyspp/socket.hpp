/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SOCKET_HPP
#define AUGSYSPP_SOCKET_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/socket.h"

namespace aug {

    inline smartfd
    socket(int domain, int type, int protocol)
    {
        smartfd sfd(smartfd::attach(aug_socket(domain, type, protocol)));
        if (null == sfd)
            throwerrinfo("aug_socket() failed");

        return sfd;
    }

    inline smartfd
    accept(fdref ref, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_accept(ref.get(), &ep)));
        if (null == sfd)
            throwerrinfo("aug_accept() failed");

        return sfd;
    }

    inline void
    bind(fdref ref, const struct aug_endpoint& ep)
    {
        if (-1 == aug_bind(ref.get(), &ep))
            throwerrinfo("aug_bind() failed");
    }

    inline void
    connect(fdref ref, const struct aug_endpoint& ep)
    {
        if (-1 == aug_connect(ref.get(), &ep))
            throwerrinfo("aug_connect() failed");
    }

    inline struct aug_endpoint&
    getpeername(fdref ref, struct aug_endpoint& ep)
    {
        if (!aug_getpeername(ref.get(), &ep))
            throwerrinfo("aug_getpeername() failed");
        return ep;
    }

    inline const struct aug_endpoint&
    getsockname(fdref ref, struct aug_endpoint& ep)
    {
        if (!aug_getsockname(ref.get(), &ep))
            throwerrinfo("aug_getsockname() failed");
        return ep;
    }

    inline void
    listen(fdref ref, int backlog)
    {
        if (-1 == aug_listen(ref.get(), backlog))
            throwerrinfo("aug_listen() failed");
    }

    inline size_t
    recv(fdref ref, void* buf, size_t len, int flags)
    {
        ssize_t ret(aug_recv(ref.get(), buf, len, flags));
        if (-1 == ret)
            throwerrinfo("aug_recv() failed");
        return ret;
    }

    inline size_t
    recvfrom(fdref ref, void* buf, size_t len, int flags,
             struct aug_endpoint& ep)
    {
        ssize_t ret(aug_recvfrom(ref.get(), buf, len, flags, &ep));
        if (-1 == ret)
            throwerrinfo("aug_recvfrom() failed");
        return ret;
    }

    inline size_t
    send(fdref ref, const void* buf, size_t len, int flags)
    {
        ssize_t ret(aug_send(ref.get(), buf, len, flags));
        if (-1 == ret)
            throwerrinfo("aug_send() failed");
        return ret;
    }

    inline size_t
    sendto(fdref ref, const void* buf, size_t len, int flags,
           const struct aug_endpoint& ep)
    {
        ssize_t ret(aug_sendto(ref.get(), buf, len, flags, &ep));
        if (-1 == ret)
            throwerrinfo("aug_sendto() failed");
        return ret;
    }

    inline void
    getsockopt(fdref ref, int level, int optname, void* optval,
               socklen_t& optlen)
    {
        if (-1 == aug_getsockopt(ref.get(), level, optname, optval, &optlen))
            throwerrinfo("aug_getsockopt() failed");
    }

    inline void
    setsockopt(fdref ref, int level, int optname, const void* optval,
               socklen_t optlen)
    {
        if (-1 == aug_setsockopt(ref.get(), level, optname, optval, optlen))
            throwerrinfo("aug_setsockopt() failed");
    }

    inline void
    shutdown(fdref ref, int how)
    {
        if (-1 == aug_shutdown(ref.get(), how))
            throwerrinfo("aug_shutdown() failed");
    }

    inline std::pair<smartfd, smartfd>
    socketpair(int domain, int type, int protocol)
    {
        int sv[2];
        if (-1 == aug_socketpair(domain, type, protocol, sv))
            throwerrinfo("aug_socketpair() failed");

        return std::make_pair(smartfd::attach(sv[0]), smartfd::attach(sv[1]));
    }

    inline std::string
    inetntop(const struct aug_ipaddr& src)
    {
        char buf[AUG_MAXADDRLEN];
        if (!aug_inetntop(&src, buf, sizeof(buf)))
            throwerrinfo("aug_inetntop() failed");
        return buf;
    }

    inline struct aug_ipaddr&
    inetpton(int af, const char* src, struct aug_ipaddr& dst)
    {
        if (!aug_inetpton(af, src, &dst))
            throwerrinfo("aug_inetpton() failed");
        return dst;
    }

    inline struct aug_ipaddr&
    inetpton(const char* src, struct aug_ipaddr& dst)
    {
        if (!aug_inetpton(AF_INET, src, &dst)
            && !aug_inetpton(AF_INET6, src, &dst))
            throwerrinfo("aug_inetpton() failed");
        return dst;
    }

    inline void
    setreuseaddr(fdref ref, bool on)
    {
        int value(on ? 1 : 0);
        if (-1 == aug_setreuseaddr(ref.get(), value))
            throwerrinfo("aug_setreuseaddr() failed");
    }

    inline int
    getfamily(fdref ref)
    {
        int ret(aug_getfamily(ref.get()));
        if (-1 == ret)
            throwerrinfo("aug_getfamily() failed");
        return ret;
    }
}

#endif // AUGSYSPP_SOCKET_HPP
