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
            if (-1 == aug_destroymuxer(muxer_))
                perrinfo(aug_tlx, "aug_destroymuxer() failed");
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
    setnowait(aug_muxer_t muxer, int nowait)
    {
        aug_setnowait(muxer, nowait);
    }

    inline void
    setfdeventmask(aug_muxer_t muxer, mdref ref, unsigned short mask)
    {
        verify(aug_setfdeventmask(muxer, ref.get(), mask));
    }

    /**
     * Returns #AUG_FAILINTR if the system call was interrupted.
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
    fdeventmask(aug_muxer_t muxer, mdref ref)
    {
        return verify(aug_fdeventmask(muxer, ref.get()));
    }

    inline unsigned short
    fdevents(aug_muxer_t muxer, mdref ref)
    {
        return verify(aug_fdevents(muxer, ref.get()));
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
