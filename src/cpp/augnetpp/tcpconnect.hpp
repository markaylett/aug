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
#ifndef AUGNETPP_TCPCONNECT_HPP
#define AUGNETPP_TCPCONNECT_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/tcpconnect.h"

#include <utility>             // pair<>

namespace aug {

    inline autosd
    tryconnect_BI(aug_tcpconnect_t conn, aug_endpoint& ep, bool& est)
    {
        int local;
        sdref ref(aug_tryconnect_BI(conn, &ep, &local));
        if (null == ref)
            throwexcept();

        // When established, aug_tryconnect() will release ownership of the
        // returned socket descriptor - it will not call aug_sclose() on it.

        if (local) {
            est = true;
            return autosd(ref, close);
        }

        est = false;
        return autosd(ref, 0);
    }

    class tcpconnect : public mpool_ops {

        aug_tcpconnect_t conn_;

        tcpconnect(const tcpconnect&);

        tcpconnect&
        operator =(const tcpconnect&);

    public:
        ~tcpconnect() AUG_NOTHROW
        {
            if (conn_)
                aug_destroytcpconnect(conn_);
        }

        tcpconnect(const null_&) AUG_NOTHROW
           : conn_(0)
        {
        }

        tcpconnect(mpoolref mpool, const char* host, const char* serv)
            : conn_(aug_createtcpconnect(mpool.get(), host, serv))
        {
            verify(conn_);
        }

        void
        swap(tcpconnect& rhs) AUG_NOTHROW
        {
            std::swap(conn_, rhs.conn_);
        }

        operator aug_tcpconnect_t()
        {
            return conn_;
        }

        aug_tcpconnect_t
        get()
        {
            return conn_;
        }
    };

    inline void
    swap(tcpconnect& lhs, tcpconnect& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }
}

inline bool
isnull(aug_tcpconnect_t tcpconnect)
{
    return !tcpconnect;
}

#endif // AUGNETPP_TCPCONNECT_HPP
