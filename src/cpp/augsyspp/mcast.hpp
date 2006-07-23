/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MCAST_HPP
#define AUGSYSPP_MCAST_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/mcast.h"

namespace aug {

    inline void
    joinmcast(fdref ref, const struct aug_inetaddr& addr, const char* ifname)
    {
        if (-1 == aug_joinmcast(ref.get(), &addr, ifname))
            throwerrinfo("aug_joinmcast() failed");
    }

    inline void
    leavemcast(fdref ref, const struct aug_inetaddr& addr, const char* ifname)
    {
        if (-1 == aug_leavemcast(ref.get(), &addr, ifname))
            throwerrinfo("aug_leavemcast() failed");
    }

    inline void
    setmcastif(fdref ref, const char* ifname)
    {
        if (-1 == aug_setmcastif(ref.get(), ifname))
            throwerrinfo("aug_setmcastif() failed");
    }

    inline void
    setmcastloop(fdref ref, int on)
    {
        if (-1 == aug_setmcastloop(ref.get(), on))
            throwerrinfo("aug_setmcastloop() failed");
    }

    inline void
    setmcasthops(fdref ref, int hops)
    {
        if (-1 == aug_setmcasthops(ref.get(), hops))
            throwerrinfo("aug_setmcasthops() failed");
    }
}

#endif // AUGSYSPP_MCAST_HPP
