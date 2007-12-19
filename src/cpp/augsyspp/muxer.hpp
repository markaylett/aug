/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MUXER_HPP
#define AUGSYSPP_MUXER_HPP

#include "augsyspp/exception.hpp"
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
            if (-1 == aug_destroymuxer(muxer_))
                perrinfo("aug_destroymuxer() failed");
        }

        muxer()
            : muxer_(aug_createmuxer())
        {
            verify(muxer_);
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
    setfdeventmask(aug_muxer_t muxer, fdref ref, unsigned short mask)
    {
        verify(aug_setfdeventmask(muxer, ref.get(), mask));
    }

    /**
     * Returns #AUG_RETINTR if the system call was interrupted.
     */

    inline int
    waitfdevents(aug_muxer_t muxer, const timeval& timeout)
    {
        return verify(aug_waitfdevents(muxer, &timeout));
    }

    inline int
    waitfdevents(aug_muxer_t muxer)
    {
        return verify(aug_waitfdevents(muxer, 0));
    }

    inline unsigned short
    fdeventmask(aug_muxer_t muxer, fdref ref)
    {
        return verify(aug_fdeventmask(muxer, ref.get()));
    }

    inline unsigned short
    fdevents(aug_muxer_t muxer, fdref ref)
    {
        return verify(aug_fdevents(muxer, ref.get()));
    }

    inline std::pair<smartfd, smartfd>
    muxerpipe()
    {
        int fds[2];
        verify(aug_muxerpipe(fds));
        return std::make_pair(smartfd::attach(fds[0]),
                              smartfd::attach(fds[1]));
    }
}

#endif // AUGSYSPP_MUXER_HPP
