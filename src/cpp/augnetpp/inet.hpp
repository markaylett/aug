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
#ifndef AUGNETPP_INET_HPP
#define AUGNETPP_INET_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"

#include "augnet/inet.h"

namespace aug {

    inline autosd
    tcpclient(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_tcpclient(host, serv, &ep), close);
        if (null == sd)
            throwerror();

        return sd;
    }

    inline autosd
    tcpserver(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_tcpserver(host, serv, &ep), close);
        if (null == sd)
            throwerror();

        return sd;
    }

    inline autosd
    udpclient(const char* host, const char* serv, aug_endpoint& ep,
              bool connect)
    {
        autosd sd(aug_udpclient(host, serv, &ep,
                                connect ? AUG_TRUE : AUG_FALSE), close);
        if (null == sd)
            throwerror();

        return sd;
    }

    inline autosd
    udpserver(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_udpserver(host, serv, &ep), close);
        if (null == sd)
            throwerror();

        return sd;
    }

    inline aug_hostserv&
    parsehostserv(const char* src, aug_hostserv& dst)
    {
        return *verify(aug_parsehostserv(src, &dst));
    }

    inline void
    setnodelay(sdref ref, bool on)
    {
        int value(on ? 1 : 0);
        verify(aug_setnodelay(ref.get(), value));
    }

    inline bool
    established(sdref ref)
    {
        aug_result result(aug_established(ref.get()));
        if (result < 0 && AUG_EXNONE == gextexcept(aug_tlx))
            return false;
        verify(result); // Possible error.
        return true;
    }
}

#endif // AUGNETPP_INET_HPP
