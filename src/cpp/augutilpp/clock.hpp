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
#ifndef AUGUTILPP_CLOCK_HPP
#define AUGUTILPP_CLOCK_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/time.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

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

    class hires : public mpool_ops {

        aug_hires_t hires_;

        hires(const hires&);

        hires&
        operator =(const hires&);

    public:
        ~hires() AUG_NOTHROW
        {
            if (hires_)
                aug_destroyhires(hires_);
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
