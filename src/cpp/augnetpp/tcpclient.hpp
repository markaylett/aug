/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_TCPCLIENT_HPP
#define AUGNETPP_TCPCLIENT_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augnet/tcpclient.h"

#include <utility> // pair<>

namespace aug {

    class tcpclient {

        aug_tcpclient_t client_;

        tcpclient(const tcpclient&);

        tcpclient&
        operator =(const tcpclient&);

    public:
        ~tcpclient() AUG_NOTHROW
        {
            if (-1 == aug_destroytcpclient(client_))
                perrinfo(aug_tlx, "aug_destroytcpclient() failed");
        }

        tcpclient(const char* host, const char* serv)
            : client_(aug_createtcpclient(host, serv))
        {
            verify(client_);
        }

        operator aug_tcpclient_t()
        {
            return client_;
        }

        aug_tcpclient_t
        get()
        {
            return client_;
        }
    };

    inline autosd
    tryconnect(aug_tcpclient_t client, aug_endpoint& ep, bool& est)
    {
        int local;
        sdref ref(aug_tryconnect(client, &ep, &local));
        if (null == ref)
            failerror();

        // When established, aug_tryconnect() will release ownership of the
        // returned socket descriptor - it will not call aug_close() on it.

        if (local) {
            est = true;
            return autosd(ref, close);
        }

        est = false;
        return autosd(ref, 0);
    }
}

#endif // AUGNETPP_TCPCLIENT_HPP
