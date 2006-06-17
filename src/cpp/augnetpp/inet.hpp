/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_INET_HPP
#define AUGNETPP_INET_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/inet.h"

namespace aug {

    AUGNETPP_API smartfd
    openpassive(const struct sockaddr_in& addr);

    inline struct sockaddr_in&
    parseinet(struct sockaddr_in& dst, const char* src)
    {
        if (!aug_parseinet(&dst, src))
            error("aug_parseinet() failed");
        return dst;
    }
}

#endif // AUGNETPP_INET_HPP
