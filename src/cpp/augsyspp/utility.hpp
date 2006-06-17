/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
        if (-1 == aug_filesize(ref.get(), &size))
            throwerror("aug_filesize() failed");
        return size;
    }

    inline void
    setnonblock(fdref ref, bool on)
    {
        if (-1 == aug_setnonblock(ref.get(), on ? 1 : 0))
            throwerror("aug_setnonblock() failed");
    }
}

#endif // AUGSYSPP_UTILITY_HPP
