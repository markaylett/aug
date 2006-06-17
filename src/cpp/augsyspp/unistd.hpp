/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UNISTD_HPP
#define AUGSYSPP_UNISTD_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/unistd.h"

namespace aug {

    inline size_t
    read(fdref ref, void* buf, size_t size)
    {
        ssize_t ret(aug_read(ref.get(), buf, size));
        if (-1 == ret)
            error("aug_read() failed");
        return ret;
    }

    inline size_t
    write(fdref ref, const void* buf, size_t size)
    {
        ssize_t ret(aug_write(ref.get(), buf, size));
        if (-1 == ret)
            error("aug_write() failed");
        return ret;
    }
}

#endif // AUGSYSPP_UNISTD_HPP
