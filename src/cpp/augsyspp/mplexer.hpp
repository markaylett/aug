/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MPLEXER_HPP
#define AUGSYSPP_MPLEXER_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/mplexer.h"

namespace aug {

    class AUGSYSPP_API mplexer {

        aug_mplexer_t mplexer_;

        mplexer(const mplexer&);

        mplexer&
        operator =(const mplexer&);

    public:
        ~mplexer() NOTHROW;

        mplexer()
            : mplexer_(aug_createmplexer())
        {
            if (!mplexer_)
                error("aug_createmplexer() failed");
        }
        void
        seteventmask(fdref ref, unsigned short mask)
        {
            if (-1 == aug_seteventmask(mplexer_, ref.get(), mask))
                error("aug_seteventmask() failed");
        }

        // Returns AUG_EINTR if the system call was interrupted.

        int
        waitevents(const struct timeval& timeout);

        int
        waitevents();

        unsigned short
        eventmask(fdref ref);

        unsigned short
        events(fdref ref);
    };

    std::pair<smartfd, smartfd>
    mplexerpipe();
}

#endif // AUGSYSPP_MPLEXER_HPP
