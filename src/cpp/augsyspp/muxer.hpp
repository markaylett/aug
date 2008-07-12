/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MUXER_HPP
#define AUGSYSPP_MUXER_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augsys/muxer.h"

#include <cerrno>

namespace aug {

    class muxer {

        aug_muxer_t muxer_;

        muxer(const muxer&);

        muxer&
        operator =(const muxer&);

    public:
        ~muxer() AUG_NOTHROW
        {
            if (muxer_ && -1 == aug_destroymuxer(muxer_))
                perrinfo(aug_tlx, "aug_destroymuxer() failed");
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

    inline void
    setmdevents(aug_muxer_t muxer, int delta)
    {
        aug_setmdevents(muxer, delta);
    }

    /**
     * Returns #AUG_FAILINTR if the system call was interrupted.
     */

    inline int
    waitmdevents(aug_muxer_t muxer, const timeval& timeout)
    {
        return verify(aug_waitmdevents(muxer, &timeout));
    }

    inline int
    waitmdevents(aug_muxer_t muxer)
    {
        return verify(aug_waitmdevents(muxer, 0));
    }

    inline unsigned short
    getmdeventmask(aug_muxer_t muxer, mdref ref)
    {
        return verify(aug_getmdeventmask(muxer, ref.get()));
    }

    inline unsigned short
    getmdevents(aug_muxer_t muxer, mdref ref)
    {
        return verify(aug_getmdevents(muxer, ref.get()));
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
