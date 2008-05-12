/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CLOCK_HPP
#define AUGUTILPP_CLOCK_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/time.hpp"
#include "augsyspp/utility.hpp" // perrinfo()

#include "augutil/hires.h"

namespace aug {

    inline void
    resethires(aug_hires_t hires)
    {
        verify(aug_resethires(hires));
    }

    inline double&
    elapsed(aug_hires_t hires, double& secs)
    {
        verify(aug_elapsed(hires, &secs));
        return secs;
    }

    inline double
    elapsed(aug_hires_t hires)
    {
        double secs;
        return elapsed(hires, secs);
    }

    class hires {

        aug_hires_t hires_;

        hires(const hires&);

        hires&
        operator =(const hires&);

    public:
        ~hires() AUG_NOTHROW
        {
            if (-1 == aug_destroyhires(hires_))
                perrinfo(aug_tlx, "aug_destroyhires() failed");
        }

        explicit
        hires(aug_mpool* mpool)
            : hires_(aug_createhires(mpool))
        {
            verify(hires_);
        }

        operator aug_hires_t()
        {
            return hires_;
        }

        aug_hires_t
        get()
        {
            return hires_;
        }
    };
}

#endif // AUGUTILPP_CLOCK_HPP
