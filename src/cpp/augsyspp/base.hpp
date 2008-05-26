/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"

#include "augctx/base.h"

#include <stdexcept>

namespace aug {

    inline void
    init()
    {
        if (aug_init() < 0)
            throw std::runtime_error("aug_init() failed");
    }
    inline void
    term()
    {
        aug_term();
    }
    inline void
    atbasixtlx()
    {
        if (aug_atbasixtlx() < 0)
            throw std::runtime_error("aug_atbasictlx() failed");
    }

    class scoped_init {

        scoped_init(const scoped_init& rhs);

        scoped_init&
        operator =(const scoped_init& rhs);

    public:
        ~scoped_init() AUG_NOTHROW
        {
            aug_term();
        }
        scoped_init()
        {
            init();
        }
    };

    inline unsigned
    nextid()
    {
        return aug_nextid();
    }
}

#endif // AUGSYSPP_BASE_HPP
