/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_PTIMER_HPP
#define AUGUTILPP_PTIMER_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/time.hpp"

#include "augutil/ptimer.h"

namespace aug {

    inline void
    resetptimer(aug_ptimer_t ptimer)
    {
        verify(aug_resetptimer(ptimer));
    }

    inline double&
    elapsed(aug_ptimer_t ptimer, double& secs)
    {
        verify(aug_elapsed(ptimer, &secs));
        return secs;
    }

    inline double
    elapsed(aug_ptimer_t ptimer)
    {
        double secs;
        elapsed(ptimer, secs);
        return secs;
    }

    class ptimer {

        aug_ptimer_t ptimer_;

        ptimer(const ptimer&);

        ptimer&
        operator =(const ptimer&);

    public:
        ~ptimer() AUG_NOTHROW
        {
            if (-1 == aug_destroyptimer(ptimer_))
                perrinfo("aug_destroyptimer() failed");
        }

        ptimer()
            : ptimer_(aug_createptimer())
        {
            verify(ptimer_);
        }

        operator aug_ptimer_t()
        {
            return ptimer_;
        }

        aug_ptimer_t
        get()
        {
            return ptimer_;
        }
    };
}

#endif // AUGUTILPP_PTIMER_HPP
