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
#ifndef AUGSYSPP_SOCKET_HPP
#define AUGSYSPP_SOCKET_HPP

#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"

#include "augsys/socket.h"

#include <string>

namespace aug {

    inline void
    close(sdref ref)
    {
        verify(aug_sclose(ref.get()));
    }

    /**
     * Set socket non-blocking on or off.
     *
     * @param ref Socket descriptor.
     *
     * @param on On or off.
     */

    inline void
    setnonblock_BI(sdref ref, bool on)
    {
        verify(aug_ssetnonblock_BI(ref.get(), on ? 1 : 0));
    }

    inline autosd
    socket(int domain, int type, int protocol = 0)
    {
        autosd sd(aug_socket(domain, type, protocol), close);
        if (null == sd)
            throwexcept();
        return sd;
    }

    inline autosd
    accept_BI(sdref ref, aug_endpoint& ep)
    {
        autosd sd(aug_accept_BI(ref.get(), &ep), close);
        if (null == sd)
            throwexcept();
        return sd;
    }

    inline void
    bind_N(sdref ref, const aug_endpoint& ep)
    {
        verify(aug_bind_N(ref.get(), &ep));
    }

    inline void
    connect_BI(sdref ref, const aug_endpoint& ep)
    {
        verify(aug_connect_BI(ref.get(), &ep));
    }

    inline aug_endpoint&
    getpeername(sdref ref, aug_endpoint& ep)
    {
        return *verify(aug_getpeername(ref.get(), &ep));
    }

    inline const aug_endpoint&
    getsockname(sdref ref, aug_endpoint& ep)
    {
        return *verify(aug_getsockname(ref.get(), &ep));
    }

    inline void
    listen(sdref ref, int backlog)
    {
        verify(aug_listen(ref.get(), backlog));
    }

    inline aug_rsize
    recv_BI(sdref ref, void* buf, size_t len, int flags)
    {
        return verify(aug_recv_BI(ref.get(), buf, len, flags));
    }

    inline aug_rsize
    recvfrom_BI(sdref ref, void* buf, size_t len, int flags,
                aug_endpoint& ep)
    {
        return verify(aug_recvfrom_BI(ref.get(), buf, len, flags, &ep));
    }

    inline aug_rsize
    send_BI(sdref ref, const void* buf, size_t len, int flags)
    {
        return verify(aug_send_BI(ref.get(), buf, len, flags));
    }

    inline aug_rsize
    sendto_BI(sdref ref, const void* buf, size_t len, int flags,
              const aug_endpoint& ep)
    {
        return verify(aug_sendto_BI(ref.get(), buf, len, flags, &ep));
    }

    inline aug_rsize
    read_BI(sdref ref, void* buf, size_t len)
    {
        return verify(aug_sread_BI(ref.get(), buf, len));
    }

    inline aug_rsize
    readv_BI(sdref ref, const iovec* iov, int size)
    {
        return verify(aug_sreadv_BI(ref.get(), iov, size));
    }

    inline aug_rsize
    write_BI(sdref ref, const void* buf, size_t len)
    {
        return verify(aug_swrite_BI(ref.get(), buf, len));
    }

    inline aug_rsize
    writev_BI(sdref ref, const iovec* iov, int size)
    {
        return verify(aug_swritev_BI(ref.get(), iov, size));
    }

    inline void
    getsockopt(sdref ref, int level, int optname, void* optval,
               socklen_t& optlen)
    {
        verify(aug_getsockopt(ref.get(), level, optname, optval, &optlen));
    }

    inline void
    setsockopt(sdref ref, int level, int optname, const void* optval,
               socklen_t optlen)
    {
        verify(aug_setsockopt(ref.get(), level, optname, optval, optlen));
    }

    inline void
    shutdown(sdref ref, int how)
    {
        verify(aug_sshutdown(ref.get(), how));
    }

    inline autosds
    socketpair_BIN(int domain, int type, int protocol)
    {
        aug_sd sv[2];
        verify(aug_socketpair_BIN(domain, type, protocol, sv));
        return autosds(sv[0], sv[1], close);
    }

    inline std::string
    endpointntop(const aug_endpoint& src)
    {
        char buf[AUG_MAXHOSTSERVLEN + 1];
        verify(aug_endpointntop(&src, buf, sizeof(buf)));
        return buf;
    }

    inline std::string
    inetntop(const aug_inetaddr& src)
    {
        char buf[AUG_MAXHOSTNAMELEN + 1];
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
            throwexcept();
        return dst;
    }

    inline int
    getfamily(sdref ref)
    {
        return verify(aug_getfamily(ref.get()));
    }

    inline void
    setreuseaddr(sdref ref, bool on)
    {
        verify(aug_setreuseaddr(ref.get(), on ? AUG_TRUE : AUG_FALSE));
    }

    inline aug_endpoint&
    getendpoint(const addrinfo& addr, aug_endpoint& ep)
    {
        return *verify(aug_getendpoint(&addr, &ep));
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
