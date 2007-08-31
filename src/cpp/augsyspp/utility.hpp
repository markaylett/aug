/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UTILITY_HPP
#define AUGSYSPP_UTILITY_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/utility.h"

namespace aug {

    inline size_t
    filesize(fdref ref)
    {
        size_t size;
        verify(aug_filesize(ref.get(), &size));
        return size;
    }

    /**
       perrinfo() can be safely called from destructors and catch blocks.
    */

    inline int
    perrinfo(const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(0, s);
    }

    inline int
    perrinfo(const aug_errinfo& errinfo, const char* s) AUG_NOTHROW
    {
        return aug_perrinfo(&errinfo, s);
    }

    inline long
    rand(void)
    {
        return aug_rand();
    }

    inline void
    srand(unsigned seed)
    {
        aug_srand(seed);
    }

    inline void
    setnonblock(fdref ref, bool on)
    {
        verify(aug_setnonblock(ref.get(), on ? 1 : 0));
    }
}

#endif // AUGSYSPP_UTILITY_HPP
