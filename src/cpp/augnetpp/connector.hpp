/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONNECTOR_HPP
#define AUGNETPP_CONNECTOR_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augnet/connector.h"

#include <utility> // pair<>

namespace aug {

    class connector {

        aug_connector_t connector_;

        connector(const connector&);

        connector&
        operator =(const connector&);

    public:
        ~connector() AUG_NOTHROW
        {
            if (-1 == aug_destroyconnector(connector_))
                perrinfo(aug_tlx, "aug_destroyconnector() failed");
        }

        connector(const char* host, const char* serv)
            : connector_(aug_createconnector(host, serv))
        {
            verify(connector_);
        }

        operator aug_connector_t()
        {
            return connector_;
        }

        aug_connector_t
        get()
        {
            return connector_;
        }
    };

    inline autosd
    tryconnect(aug_connector_t ctor, aug_endpoint& ep, bool& est)
    {
        int local;
        sdref ref(aug_tryconnect(ctor, &ep, &local));
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

#endif // AUGNETPP_CONNECTOR_HPP
