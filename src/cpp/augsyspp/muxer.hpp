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
#ifndef AUGSYSPP_MUXER_HPP
#define AUGSYSPP_MUXER_HPP

#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augsys/muxer.h"

#include <cerrno>

namespace aug {

    class muxer : public mpool_ops {

        aug_muxer_t muxer_;

        muxer(const muxer&);

        muxer&
        operator =(const muxer&);

    public:
        ~muxer() AUG_NOTHROW
        {
            if (muxer_)
                aug_destroymuxer(muxer_);
        }

        muxer(const null_&) AUG_NOTHROW
           : muxer_(0)
        {
        }

        explicit
        muxer(mpoolref mpool)
            : muxer_(aug_createmuxer(mpool.get()))
        {
            verify(muxer_);
        }

        void
        swap(muxer& rhs) AUG_NOTHROW
        {
            std::swap(muxer_, rhs.muxer_);
        }

        operator aug_muxer_t()
        {
            return muxer_;
        }

        aug_muxer_t
        get()
        {
            return muxer_;
        }
    };

    inline void
    swap(muxer& lhs, muxer& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline void
    setmdeventmask(aug_muxer_t muxer, mdref ref, unsigned short mask)
    {
        verify(aug_setmdeventmask(muxer, ref.get(), mask));
    }

    /**
     * Throws intr_exception if the system call was interrupted.
     */

    inline unsigned
    waitmdevents(aug_muxer_t muxer, const timeval& timeout)
    {
        return AUG_RESULT(verify(aug_waitmdevents(muxer, &timeout)));
    }

    /**
     * Throws intr_exception if the system call was interrupted.
     */

    inline unsigned
    waitmdevents(aug_muxer_t muxer)
    {
        return AUG_RESULT(verify(aug_waitmdevents(muxer, 0)));
    }

    inline unsigned
    pollmdevents(aug_muxer_t muxer)
    {
        return AUG_RESULT(verify(aug_pollmdevents(muxer)));
    }

    inline unsigned short
    getmdeventmask(aug_muxer_t muxer, mdref ref)
    {
        return aug_getmdeventmask(muxer, ref.get());
    }

    inline unsigned short
    getmdevents(aug_muxer_t muxer, mdref ref)
    {
        return aug_getmdevents(muxer, ref.get());
    }

    inline automds
    muxerpipe()
    {
        aug_md mds[2];
        verify(aug_muxerpipe(mds));
        return automds(mds[0], mds[1], close);
    }
}

#endif // AUGSYSPP_MUXER_HPP
