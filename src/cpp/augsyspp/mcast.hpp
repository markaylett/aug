/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MCAST_HPP
#define AUGSYSPP_MCAST_HPP

#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"

#include "augsys/mcast.h"

namespace aug {

    inline void
    joinmcast(sdref ref, const aug_inetaddr& addr, const char* ifname)
    {
        verify(aug_joinmcast(ref.get(), &addr, ifname));
    }

    inline void
    leavemcast(sdref ref, const aug_inetaddr& addr, const char* ifname)
    {
        verify(aug_leavemcast(ref.get(), &addr, ifname));
    }

    inline void
    setmcastif(sdref ref, const char* ifname)
    {
        verify(aug_setmcastif(ref.get(), ifname));
    }

    inline void
    setmcastloop(sdref ref, bool on)
    {
        verify(aug_setmcastloop(ref.get(), on ? AUG_TRUE : AUG_FALSE));
    }

    inline void
    setmcastttl(sdref ref, int ttl)
    {
        verify(aug_setmcastttl(ref.get(), ttl));
    }
}

#endif // AUGSYSPP_MCAST_HPP
