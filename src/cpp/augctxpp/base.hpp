/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_BASE_HPP
#define AUGCTXPP_BASE_HPP

#include "augctx/base.h"

#include <stdexcept>

namespace aug {

    const struct basictlx_ { } basictlx = basictlx_();

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
    initbasictlx()
    {
        if (aug_initbasictlx() < 0)
            throw std::runtime_error("aug_initbasictlx() failed");
    }
    inline void
    autobasictlx()
    {
        if (aug_autobasictlx() < 0)
            throw std::runtime_error("aug_autobasictlx() failed");
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
        scoped_init(const basictlx_&)
        {
            initbasictlx();
        }
    };
}

#endif // AUGCTXPP_BASE_HPP
