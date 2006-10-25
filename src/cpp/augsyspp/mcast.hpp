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
        verify(aug_joinmcast(ref.get(), &addr, ifname));
    }

    inline void
    leavemcast(fdref ref, const struct aug_inetaddr& addr, const char* ifname)
    {
        verify(aug_leavemcast(ref.get(), &addr, ifname));
    }

    inline void
    setmcastif(fdref ref, const char* ifname)
    {
        verify(aug_setmcastif(ref.get(), ifname));
    }

    inline void
    setmcastloop(fdref ref, int on)
    {
        verify(aug_setmcastloop(ref.get(), on));
    }

    inline void
    setmcastttl(fdref ref, int ttl)
    {
        verify(aug_setmcastttl(ref.get(), ttl));
    }
}

#endif // AUGSYSPP_MCAST_HPP
