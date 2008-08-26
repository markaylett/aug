/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CLOCK_HPP
#define AUGUTILPP_CLOCK_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/time.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

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
            if (hires_ && -1 == aug_destroyhires(hires_))
                perrinfo(aug_tlx, "aug_destroyhires() failed");
        }

        hires(const null_&) AUG_NOTHROW
           : hires_(0)
        {
        }

        explicit
        hires(mpoolref mpool)
            : hires_(aug_createhires(mpool.get()))
        {
            verify(hires_);
        }

        void
        swap(hires& rhs) AUG_NOTHROW
        {
            std::swap(hires_, rhs.hires_);
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

    inline void
    swap(hires& lhs, hires& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }
}

#endif // AUGUTILPP_CLOCK_HPP
