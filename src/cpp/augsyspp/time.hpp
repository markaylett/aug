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
#ifndef AUGSYSPP_TIME_HPP
#define AUGSYSPP_TIME_HPP

#include "augctxpp/exception.hpp"

#include "augsys/time.h"

namespace aug {

    inline aug_rlong
    timegm(tm& res)
    {
        return verify(aug_timegm(&res));
    }

    inline aug_rlong
    timelocal(tm& res)
    {
        return verify(aug_timelocal(&res));
    }

    inline tm&
    gmtime(const aug_time& clock, tm& res)
    {
        return *verify(aug_gmtime(&clock, &res));
    }

    inline tm&
    gmtime(tm& res)
    {
        return gmtime(static_cast<aug_time>(time(0)), res);
    }

    inline tm&
    localtime(const aug_time& clock, tm& res)
    {
        return *verify(aug_localtime(&clock, &res));
    }

    inline tm&
    localtime(tm& res)
    {
        return localtime(static_cast<aug_time>(time(0)), res);
    }

    inline aug_timeval&
    mstotv(unsigned ms, aug_timeval& tv)
    {
        return *aug_mstotv(ms, &tv);
    }

    inline unsigned
    tvtoms(const aug_timeval& tv)
    {
        return aug_tvtoms(&tv);
    }

    inline aug_timeval&
    tvadd(aug_timeval& dst, const aug_timeval& src)
    {
        return *aug_tvadd(&dst, &src);
    }

    inline aug_timeval&
    tvsub(aug_timeval& dst, const aug_timeval& src)
    {
        return *aug_tvsub(&dst, &src);
    }
}

#endif // AUGSYSPP_TIME_HPP
