/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TIME_HPP
#define AUGSYSPP_TIME_HPP

#include "augsyspp/exception.hpp"

#include "augsys/time.h"

namespace aug {

    inline struct timeval&
    gettimeofday(struct timeval& tv, struct timezone& tz)
    {
        if (-1 == aug_gettimeofday(&tv, &tz))
            throwerrinfo("aug_gettimeofday() failed");
        return tv;
    }

    inline struct timeval&
    gettimeofday(struct timeval& tv)
    {
        if (-1 == aug_gettimeofday(&tv, 0))
            throwerrinfo("aug_gettimeofday() failed");
        return tv;
    }

    inline struct tm&
    localtime(const time_t& clock, struct tm& res)
    {
        return *aug_localtime(&clock, &res);
    }

    inline struct timeval&
    mstotv(struct timeval& tv, unsigned ms)
    {
        return *aug_mstotv(&tv, ms);
    }

    inline unsigned
    tvtoms(const struct timeval& tv)
    {
        return aug_tvtoms(&tv);
    }

    inline struct timeval&
    tvadd(struct timeval& dst, const struct timeval& src)
    {
        return *aug_tvadd(&dst, &src);
    }

    inline struct timeval&
    tvsub(struct timeval& dst, const struct timeval& src)
    {
        return *aug_tvsub(&dst, &src);
    }
}

#endif // AUGSYSPP_TIME_HPP
