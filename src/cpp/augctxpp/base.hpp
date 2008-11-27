/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_BASE_HPP
#define AUGCTXPP_BASE_HPP

#include "augctxpp/exception.hpp"

#include "augctx/base.h"

#include <stdexcept>

namespace aug {

    inline void
    init()
    {
        if (!aug_init())
            throw std::runtime_error("aug_init() failed");
    }
    inline void
    term()
    {
        aug_term();
    }
    inline void
    setbasictlx(mpoolref mpool)
    {
        verify(aug_setbasictlx(mpool.get()));
    }
    inline void
    inittlx()
    {
        if (!aug_inittlx())
            throw std::runtime_error("aug_inittlx() failed");
    }
    inline void
    autotlx()
    {
        if (!aug_autotlx())
            throw std::runtime_error("aug_autotlx() failed");
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
        explicit
        scoped_init(const null_&)
        {
            init();
        }
        explicit
        scoped_init(const tlx_&)
        {
            inittlx();
        }
    };
}

#endif // AUGCTXPP_BASE_HPP
