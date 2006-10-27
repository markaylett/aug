/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TIME_HPP
#define AUGSYSPP_TIME_HPP

#include "augsyspp/exception.hpp"

#include "augsys/time.h"

namespace aug {

    inline timeval&
    gettimeofday(timeval& tv, struct timezone& tz)
    {
        verify(aug_gettimeofday(&tv, &tz));
        return tv;
    }

    inline timeval&
    gettimeofday(timeval& tv)
    {
        verify(aug_gettimeofday(&tv, 0));
        return tv;
    }

    inline tm&
    localtime(const time_t& clock, tm& res)
    {
        return *aug_localtime(&clock, &res);
    }

    inline timeval&
    mstotv(timeval& tv, unsigned ms)
    {
        return *aug_mstotv(&tv, ms);
    }

    inline unsigned
    tvtoms(const timeval& tv)
    {
        return aug_tvtoms(&tv);
    }

    inline timeval&
    tvadd(timeval& dst, const timeval& src)
    {
        return *aug_tvadd(&dst, &src);
    }

    inline timeval&
    tvsub(timeval& dst, const timeval& src)
    {
        return *aug_tvsub(&dst, &src);
    }
}

#endif // AUGSYSPP_TIME_HPP
