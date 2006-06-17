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
            error("aug_socket() failed");

        return sfd;
    }

    inline smartfd
    accept(fdref ref, struct sockaddr& addr, socklen_t& addrlen)
    {
        smartfd sfd(smartfd::attach(aug_accept(ref.get(), &addr, &addrlen)));
        if (null == sfd)
            error("aug_accept() failed");

        return sfd;
    }

    inline void
    bind(fdref ref, const struct sockaddr& addr, socklen_t addrlen)
    {
        if (-1 == aug_bind(ref.get(), &addr, addrlen))
            error("aug_bind() failed");
    }

    inline void
    connect(fdref ref, const struct sockaddr& addr, socklen_t addrlen)
    {
        if (-1 == aug_connect(ref.get(), &addr, addrlen))
            error("aug_connect() failed");
    }

    inline void
    getpeername(fdref ref, struct sockaddr& addr, socklen_t& addrlen)
    {
        if (-1 == aug_getpeername(ref.get(), &addr, &addrlen))
            error("aug_getpeername() failed");
    }

    inline void
    getsockname(fdref ref, struct sockaddr& addr, socklen_t& addrlen)
    {
        if (-1 == aug_getsockname(ref.get(), &addr, &addrlen))
            error("aug_getsockname() failed");
    }

    inline void
    listen(fdref ref, int backlog)
    {
        if (-1 == aug_listen(ref.get(), backlog))
            error("aug_listen() failed");
    }

    inline size_t
    recv(fdref ref, void* buf, size_t len, int flags)
    {
        ssize_t ret(aug_recv(ref.get(), buf, len, flags));
        if (-1 == ret)
            error("aug_recv() failed");
        return ret;
    }

    inline size_t
    recvfrom(fdref ref, void* buf, size_t len, int flags,
             struct sockaddr& from, socklen_t& fromlen)
    {
        ssize_t ret(aug_recvfrom(ref.get(), buf, len, flags, &from,
                                 &fromlen));
        if (-1 == ret)
            error("aug_recvfrom() failed");
        return ret;
    }

    inline size_t
    send(fdref ref, const void* buf, size_t len, int flags)
    {
        ssize_t ret(aug_send(ref.get(), buf, len, flags));
        if (-1 == ret)
            error("aug_send() failed");
        return ret;
    }

    inline size_t
    sendto(fdref ref, const void* buf, size_t len, int flags,
           const struct sockaddr& to, socklen_t tolen)
    {
        ssize_t ret(aug_sendto(ref.get(), buf, len, flags, &to, tolen));
        if (-1 == ret)
            error("aug_sendto() failed");
        return ret;
    }

    inline void
    getsockopt(fdref ref, int level, int optname, void* optval,
               socklen_t& optlen)
    {
        if (-1 == aug_getsockopt(ref.get(), level, optname, optval, &optlen))
            error("aug_getsockopt() failed");
    }

    inline void
    setsockopt(fdref ref, int level, int optname, const void* optval,
               socklen_t optlen)
    {
        if (-1 == aug_setsockopt(ref.get(), level, optname, optval, optlen))
            error("aug_setsockopt() failed");
    }

    inline void
    shutdown(fdref ref, int how)
    {
        if (-1 == aug_shutdown(ref.get(), how))
            error("aug_shutdown() failed");
    }

    inline std::pair<smartfd, smartfd>
    socketpair(int domain, int type, int protocol)
    {
        int sv[2];
        if (-1 == aug_socketpair(domain, type, protocol, sv))
            error("aug_socketpair() failed");

        return std::make_pair(smartfd::attach(sv[0]), smartfd::attach(sv[1]));
    }
}

#endif // AUGSYSPP_SOCKET_HPP
