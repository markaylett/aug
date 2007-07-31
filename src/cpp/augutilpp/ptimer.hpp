/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_PTIMER_HPP
#define AUGUTILPP_PTIMER_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/ptimer.h"

namespace aug {

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

        unsigned long
        now() const
        {
            return aug_ptimernow(ptimer_);
        }
    };
}

#endif // AUGUTILPP_PTIMER_HPP
