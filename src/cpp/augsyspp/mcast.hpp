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
