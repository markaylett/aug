/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MPLEXER_HPP
#define AUGSYSPP_MPLEXER_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/mplexer.h"
#include "augsys/string.h" // aug_perror()

#include <cerrno>

namespace aug {

    class AUGSYSPP_API mplexer {

        aug_mplexer_t mplexer_;

        mplexer(const mplexer&);

        mplexer&
        operator =(const mplexer&);

    public:
        ~mplexer() NOTHROW
        {
            if (-1 == aug_freemplexer(mplexer_))
                aug_perror("aug_freemplexer() failed");
        }

        mplexer()
            : mplexer_(aug_createmplexer())
        {
            if (!mplexer_)
                throwerror("aug_createmplexer() failed");
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
    seteventmask(aug_mplexer_t mplexer, fdref ref, unsigned short mask)
    {
        if (-1 == aug_seteventmask(mplexer, ref.get(), mask))
            throwerror("aug_seteventmask() failed");
    }

    /** Returns AUG_RETINTR if the system call was interrupted. */

    inline int
    waitevents(aug_mplexer_t mplexer, const struct timeval& timeout)
    {
        int ret(aug_waitevents(mplexer, &timeout));
        if (-1 == ret)
            throwerror("aug_waitevents() failed");
        return ret;
    }

    inline int
    waitevents(aug_mplexer_t mplexer)
    {
        int ret(aug_waitevents(mplexer, 0));
        if (-1 == ret)
            throwerror("aug_waitevents() failed");
        return ret;
    }

    inline unsigned short
    eventmask(aug_mplexer_t mplexer, fdref ref)
    {
        int ret(aug_eventmask(mplexer, ref.get()));
        if (-1 == ret)
            throwerror("aug_eventmask() failed");

        return ret;
    }

    inline unsigned short
    events(aug_mplexer_t mplexer, fdref ref)
    {
        int ret(aug_events(mplexer, ref.get()));
        if (-1 == ret)
            throwerror("aug_events() failed");

        return ret;
    }

    inline std::pair<smartfd, smartfd>
    mplexerpipe()
    {
        int fds[2];
        if (-1 == aug_mplexerpipe(fds))
            throwerror("aug_mplexerpipe() failed");

        return std::make_pair(smartfd::attach(fds[0]),
                              smartfd::attach(fds[1]));
    }
}

#endif // AUGSYSPP_MPLEXER_HPP
