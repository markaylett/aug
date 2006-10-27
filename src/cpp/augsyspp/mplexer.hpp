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
                perrinfo("aug_freemplexer() failed");
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
    setioeventmask(aug_mplexer_t mplexer, fdref ref, unsigned short mask)
    {
        verify(aug_setioeventmask(mplexer, ref.get(), mask));
    }

    /**
       Returns #AUG_RETINTR if the system call was interrupted.
    */

    inline int
    waitioevents(aug_mplexer_t mplexer, const timeval& timeout)
    {
        return verify(aug_waitioevents(mplexer, &timeout));
    }

    inline int
    waitioevents(aug_mplexer_t mplexer)
    {
        return verify(aug_waitioevents(mplexer, 0));
    }

    inline unsigned short
    ioeventmask(aug_mplexer_t mplexer, fdref ref)
    {
        return verify(aug_ioeventmask(mplexer, ref.get()));
    }

    inline unsigned short
    ioevents(aug_mplexer_t mplexer, fdref ref)
    {
        return verify(aug_ioevents(mplexer, ref.get()));
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
