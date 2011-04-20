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
#ifndef AUGNETPP_CHANS_HPP
#define AUGNETPP_CHANS_HPP

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augnet/chans.h"

namespace aug {

    inline void
    insertchan(aug_chans_t chans, chanref chan)
    {
        verify(aug_insertchan(chans, chan.get()));
    }

    inline void
    removechan(aug_chans_t chans, aug_id id)
    {
        verify(aug_removechan(chans, id));
    }

    inline chanptr
    findchan(aug_chans_t chans, aug_id id)
    {
        return object_attach<aug_chan>(aug_findchan(chans, id));
    }

    inline void
    processchans_BI(aug_chans_t chans)
    {
        aug_processchans_BI(chans);
    }

    inline void
    dumpchans(aug_chans_t chans)
    {
        aug_dumpchans(chans);
    }

    inline unsigned
    getchans(aug_chans_t chans)
    {
        return aug_getchans(chans);
    }

    inline unsigned
    getreadychans(aug_chans_t chans)
    {
        return aug_getreadychans(chans);
    }

    class chans : public mpool_ops {

        aug_chans_t chans_;

        chans(const chans&);

        chans&
        operator =(const chans&);

    public:
        ~chans() AUG_NOTHROW
        {
            if (chans_)
                aug_destroychans(chans_);
        }

        chans(const null_&) AUG_NOTHROW
           : chans_(0)
        {
        }

        chans(mpoolref mpool, chandlerref chandler)
            : chans_(aug_createchans(mpool.get(), chandler.get()))
        {
            verify(chans_);
        }

        void
        swap(chans& rhs) AUG_NOTHROW
        {
            std::swap(chans_, rhs.chans_);
        }

        operator aug_chans_t()
        {
            return chans_;
        }

        aug_chans_t
        get()
        {
            return chans_;
        }
    };

    inline void
    swap(chans& lhs, chans& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }
}

inline bool
isnull(aug_chans_t chans)
{
    return !chans;
}

#endif // AUGNETPP_CHANS_HPP
