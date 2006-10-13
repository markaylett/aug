/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
            if (-1 == aug_freemplexer(mplexer_))
                aug_perrinfo("aug_freemplexer() failed");
        }

        mplexer()
            : mplexer_(aug_createmplexer())
        {
            if (!mplexer_)
                throwerrinfo("aug_createmplexer() failed");
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
    setioeventmask(aug_mplexer_t mplexer, fdref ref, unsigned short mask)
    {
        if (-1 == aug_setioeventmask(mplexer, ref.get(), mask))
            throwerrinfo("aug_setioeventmask() failed");
    }

    /**
       Returns #AUG_RETINTR if the system call was interrupted.
    */

    inline int
    waitioevents(aug_mplexer_t mplexer, const struct timeval& timeout)
    {
        int ret(aug_waitioevents(mplexer, &timeout));
        if (-1 == ret)
            throwerrinfo("aug_waitioevents() failed");
        return ret;
    }

    inline int
    waitioevents(aug_mplexer_t mplexer)
    {
        int ret(aug_waitioevents(mplexer, 0));
        if (-1 == ret)
            throwerrinfo("aug_waitioevents() failed");
        return ret;
    }

    inline unsigned short
    ioeventmask(aug_mplexer_t mplexer, fdref ref)
    {
        int ret(aug_ioeventmask(mplexer, ref.get()));
        if (-1 == ret)
            throwerrinfo("aug_ioeventmask() failed");

        return ret;
    }

    inline unsigned short
    ioevents(aug_mplexer_t mplexer, fdref ref)
    {
        int ret(aug_ioevents(mplexer, ref.get()));
        if (-1 == ret)
            throwerrinfo("aug_ioevents() failed");

        return ret;
    }

    inline std::pair<smartfd, smartfd>
    mplexerpipe()
    {
        int fds[2];
        if (-1 == aug_mplexerpipe(fds))
            throwerrinfo("aug_mplexerpipe() failed");

        return std::make_pair(smartfd::attach(fds[0]),
                              smartfd::attach(fds[1]));
    }
}

#endif // AUGSYSPP_MPLEXER_HPP
