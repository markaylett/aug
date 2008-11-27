/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_BASE_HPP
#define AUGCTXPP_BASE_HPP

#include "augctxpp/exception.hpp"

#include "augctx/base.h"

#include <stdexcept>

namespace aug {

    const struct dltlx_ { } dltlx = dltlx_();

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
    initdltlx()
    {
        if (!aug_initdltlx())
            throw std::runtime_error("aug_initdltlx() failed");
    }
    inline void
    autodltlx()
    {
        if (!aug_autodltlx())
            throw std::runtime_error("aug_autodltlx() failed");
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
        scoped_init(const dltlx_&)
        {
            initdltlx();
        }
    };
}

#endif // AUGCTXPP_BASE_HPP
