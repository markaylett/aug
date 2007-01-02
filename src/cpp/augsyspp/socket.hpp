/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_SOCKET_HPP
#define AUGSYSPP_SOCKET_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/socket.h"

namespace aug {

    inline smartfd
    socket(int domain, int type, int protocol = 0)
    {
        smartfd sfd(smartfd::attach(aug_socket(domain, type, protocol)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline smartfd
    accept(fdref ref, aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_accept(ref.get(), &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline void
    bind(fdref ref, const aug_endpoint& ep)
    {
        verify(aug_bind(ref.get(), &ep));
    }

    inline void
    connect(fdref ref, const aug_endpoint& ep)
    {
        verify(aug_connect(ref.get(), &ep));
    }

    inline aug_endpoint&
    getpeername(fdref ref, aug_endpoint& ep)
    {
        return *verify(aug_getpeername(ref.get(), &ep));
    }

    inline const aug_endpoint&
    getsockname(fdref ref, aug_endpoint& ep)
    {
        return *verify(aug_getsockname(ref.get(), &ep));
    }

    inline void
    listen(fdref ref, int backlog)
    {
        verify(aug_listen(ref.get(), backlog));
    }

    inline size_t
    recv(fdref ref, void* buf, size_t len, int flags)
    {
        return verify(aug_recv(ref.get(), buf, len, flags));
    }

    inline size_t
    recvfrom(fdref ref, void* buf, size_t len, int flags,
             aug_endpoint& ep)
    {
        return verify(aug_recvfrom(ref.get(), buf, len, flags, &ep));
    }

    inline size_t
    send(fdref ref, const void* buf, size_t len, int flags)
    {
        return verify(aug_send(ref.get(), buf, len, flags));
    }

    inline size_t
    sendto(fdref ref, const void* buf, size_t len, int flags,
           const aug_endpoint& ep)
    {
        return verify(aug_sendto(ref.get(), buf, len, flags, &ep));
    }

    inline void
    getsockopt(fdref ref, int level, int optname, void* optval,
               socklen_t& optlen)
    {
        verify(aug_getsockopt(ref.get(), level, optname, optval, &optlen));
    }

    inline void
    setsockopt(fdref ref, int level, int optname, const void* optval,
               socklen_t optlen)
    {
        verify(aug_setsockopt(ref.get(), level, optname, optval, optlen));
    }

    inline void
    shutdown(fdref ref, int how)
    {
        verify(aug_shutdown(ref.get(), how));
    }

    inline std::pair<smartfd, smartfd>
    socketpair(int domain, int type, int protocol)
    {
        int sv[2];
        verify(aug_socketpair(domain, type, protocol, sv));
        return std::make_pair(smartfd::attach(sv[0]), smartfd::attach(sv[1]));
    }

    inline std::string
    inetntop(const aug_inetaddr& src)
    {
        char buf[AUG_MAXADDRLEN];
        verify(aug_inetntop(&src, buf, sizeof(buf)));
        return buf;
    }

    inline aug_inetaddr&
    inetpton(int af, const char* src, aug_inetaddr& dst)
    {
        return *verify(aug_inetpton(af, src, &dst));
    }

    inline aug_inetaddr&
    inetpton(const char* src, aug_inetaddr& dst)
    {
        if (!aug_inetpton(AF_INET, src, &dst)
            && !aug_inetpton(AF_INET6, src, &dst))
            fail();
        return dst;
    }

    inline int
    getfamily(fdref ref)
    {
        return verify(aug_getfamily(ref.get()));
    }

    inline void
    setreuseaddr(fdref ref, bool on)
    {
        int value(on ? 1 : 0);
        verify(aug_setreuseaddr(ref.get(), value));
    }

    inline aug_endpoint&
    setinetaddr(aug_endpoint& ep, const aug_inetaddr& addr)
    {
        return *verify(aug_setinetaddr(&ep, &addr));
    }

    inline aug_inetaddr&
    getinetaddr(const aug_endpoint& ep, aug_inetaddr& addr)
    {
        return *verify(aug_getinetaddr(&ep, &addr));
    }

    inline const aug_inetaddr&
    inetany(int af)
    {
        return *verify(aug_inetany(af));
    }

    inline const aug_inetaddr&
    inetloopback(int af)
    {
        return *verify(aug_inetloopback(af));
    }
}

#endif // AUGSYSPP_SOCKET_HPP
