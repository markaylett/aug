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
#ifndef AUGMARPP_SMARTMAR_HPP
#define AUGMARPP_SMARTMAR_HPP

#include "augmarpp/types.hpp"

#include "augctxpp/exception.hpp"

#include <algorithm>            // swap()

namespace aug {

    class smartmar {
        marref ref_;

        smartmar(marref ref, bool retain)
            : ref_(ref)
        {
            if (retain && null != ref)
                aug_retainmar(ref.get());
        }

    public:
        ~smartmar() AUG_NOTHROW
        {
            if (null != ref_)
                aug_releasemar(ref_.get());
        }

        smartmar(const null_&) AUG_NOTHROW
        : ref_(null)
        {
        }

        smartmar(const smartmar& rhs)
            : ref_(rhs.ref_)
        {
            if (null != ref_)
                aug_retainmar(ref_.get());
        }

        smartmar&
        operator =(const null_& rhs) AUG_NOTHROW
        {
            ref_ = null;
            return *this;
        }

        smartmar&
        operator =(const smartmar& rhs)
        {
            smartmar tmp(rhs);
            swap(tmp);
            return *this;
        }

        void
        swap(smartmar& rhs) AUG_NOTHROW
        {
            std::swap(ref_, rhs.ref_);
        }

        void
        release()
        {
            if (null != ref_) {
                marref ref(ref_);
                ref_ = null;
                aug_releasemar(ref.get());
            }
        }

        static smartmar
        attach(aug_mar_t mar)
        {
            return smartmar(mar, false);
        }

        static smartmar
        retain(aug_mar_t mar)
        {
            return smartmar(mar, true);
        }

        operator marref() const
        {
            return ref_;
        }

        aug_mar_t
        get() const
        {
            return ref_.get();
        }
    };
}

inline bool
isnull(const aug::smartmar& mar)
{
    return 0 == mar.get();
}

#endif // AUGMARPP_SMARTMAR_HPP
