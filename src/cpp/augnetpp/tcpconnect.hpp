/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_TCPCONNECT_HPP
#define AUGNETPP_TCPCONNECT_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"

#include "augnet/tcpconnect.h"

#include <utility>             // pair<>

namespace aug {

    class tcpconnect {

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

    inline autosd
    tryconnect(aug_tcpconnect_t conn, aug_endpoint& ep, bool& est)
    {
        int local;
        sdref ref(aug_tryconnect(conn, &ep, &local));
        if (null == ref)
            throwerror();

        // When established, aug_tryconnect() will release ownership of the
        // returned socket descriptor - it will not call aug_sclose() on it.

        if (local) {
            est = true;
            return autosd(ref, close);
        }

        est = false;
        return autosd(ref, 0);
    }
}

#endif // AUGNETPP_TCPCONNECT_HPP
