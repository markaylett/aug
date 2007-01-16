/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONNECTOR_HPP
#define AUGNETPP_CONNECTOR_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
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
            if (-1 == aug_freeconnector(connector_))
                perrinfo("aug_freeconnector() failed");
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

    inline std::pair<smartfd, bool>
    tryconnect(aug_connector_t ctor, aug_endpoint& ep)
    {
        int est;
        int ret(verify(aug_tryconnect(ctor, &ep, &est)));
        if (est)
            return std::make_pair(smartfd::attach(ret), true);

        return std::make_pair(smartfd::retain(ret), false);
    }
}

#endif // AUGNETPP_CONNECTOR_HPP
