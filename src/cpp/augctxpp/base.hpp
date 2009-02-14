/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
