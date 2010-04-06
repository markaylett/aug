/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGSYSPP_MMAP_HPP
#define AUGSYSPP_MMAP_HPP

#include "augsyspp/types.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augsys/mmap.h"

namespace aug {

    inline void
    remmap_BIN(aug_mmap& mm, size_t offset, size_t len)
    {
        verify(aug_remmap_BIN(&mm, offset, len));
    }

    inline void
    syncmmap(const aug_mmap& mm)
    {
        verify(aug_syncmmap(&mm));
    }

    inline size_t
    mmapsize(const aug_mmap& mm)
    {
        return aug_mmapsize(&mm);
    }

    class mmap_BIN : public mpool_ops {
    public:
        typedef aug_mmap ctype;
    private:
        aug_mmap* const mmap_;

        mmap_BIN(const mmap_BIN&);

        mmap_BIN&
        operator =(const mmap_BIN&);

    public:
        ~mmap_BIN() AUG_NOTHROW
        {
            aug_destroymmap_B(mmap_);
        }

        mmap_BIN(mpoolref mpool, fdref ref, size_t offset, size_t len,
                 int flags)
            : mmap_(aug_createmmap_BIN(mpool.get(), ref.get(), offset, len,
                                       flags))
        {
            verify(mmap_);
        }

        void*
        addr() const
        {
            return mmap_->addr_;
        }

        size_t
        len() const
        {
            return mmap_->len_;
        }

        operator aug_mmap&()
        {
            return *mmap_;
        }

        operator const aug_mmap&() const
        {
            return *mmap_;
        }
    };
}

#endif // AUGSYSPP_MMAP_HPP
