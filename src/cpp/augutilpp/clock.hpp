/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CLOCK_HPP
#define AUGUTILPP_CLOCK_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/time.hpp"
#include "augsyspp/utility.hpp" // perrinfo()

#include "augutil/clock.h"

namespace aug {

    inline void
    resetclock(aug_clock_t clock)
    {
        verify(aug_resetclock(clock));
    }

    inline double&
    elapsed(aug_clock_t clock, double& secs)
    {
        verify(aug_elapsed(clock, &secs));
        return secs;
    }

    inline double
    elapsed(aug_clock_t clock)
    {
        double secs;
        return elapsed(clock, secs);
    }

    class clock {

        aug_clock_t clock_;

        clock(const clock&);

        clock&
        operator =(const clock&);

    public:
        ~clock() AUG_NOTHROW
        {
            if (-1 == aug_destroyclock(clock_))
                perrinfo("aug_destroyclock() failed");
        }

        clock()
            : clock_(aug_createclock())
        {
            verify(clock_);
        }

        operator aug_clock_t()
        {
            return clock_;
        }

        aug_clock_t
        get()
        {
            return clock_;
        }
    };
}

#endif // AUGUTILPP_CLOCK_HPP
