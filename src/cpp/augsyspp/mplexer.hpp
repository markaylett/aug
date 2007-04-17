/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MPLEXER_HPP
#define AUGSYSPP_MPLEXER_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/mplexer.h"

#include <cerrno>

namespace aug {

    class mplexer {

        aug_mplexer_t mplexer_;

        mplexer(const mplexer&);

        mplexer&
        operator =(const mplexer&);

    public:
        ~mplexer() AUG_NOTHROW
        {
            if (-1 == aug_destroymplexer(mplexer_))
                perrinfo("aug_destroymplexer() failed");
        }

        mplexer()
            : mplexer_(aug_createmplexer())
        {
            verify(mplexer_);
        }

        operator aug_mplexer_t()
        {
            return mplexer_;
        }

        aug_mplexer_t
        get()
        {
            return mplexer_;
        }
    };

    inline void
    setfdeventmask(aug_mplexer_t mplexer, fdref ref, unsigned short mask)
    {
        verify(aug_setfdeventmask(mplexer, ref.get(), mask));
    }

    /**
       Returns #AUG_RETINTR if the system call was interrupted.
    */

    inline int
    waitfdevents(aug_mplexer_t mplexer, const timeval& timeout)
    {
        return verify(aug_waitfdevents(mplexer, &timeout));
    }

    inline int
    waitfdevents(aug_mplexer_t mplexer)
    {
        return verify(aug_waitfdevents(mplexer, 0));
    }

    inline unsigned short
    fdeventmask(aug_mplexer_t mplexer, fdref ref)
    {
        return verify(aug_fdeventmask(mplexer, ref.get()));
    }

    inline unsigned short
    fdevents(aug_mplexer_t mplexer, fdref ref)
    {
        return verify(aug_fdevents(mplexer, ref.get()));
    }

    inline std::pair<smartfd, smartfd>
    mplexerpipe()
    {
        int fds[2];
        verify(aug_mplexerpipe(fds));
        return std::make_pair(smartfd::attach(fds[0]),
                              smartfd::attach(fds[1]));
    }
}

#endif // AUGSYSPP_MPLEXER_HPP
