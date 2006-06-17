/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_SIGNAL_HPP
#define AUGSRVPP_SIGNAL_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/signal.h"

namespace aug {

    inline void
    signalhandler(void (*handler)(int))
    {
        if (-1 == aug_signalhandler(handler))
            throwerrinfo("aug_signalhandler() failed");
    }

    inline void
    blocksignals()
    {
        if (-1 == aug_blocksignals())
            throwerrinfo("aug_blocksignals() failed");
    }

    inline void
    unblocksignals()
    {
        if (-1 == aug_unblocksignals())
            throwerrinfo("aug_unblocksignals() failed");
    }

    class AUGSYSPP_API scoped_unblock {

        scoped_unblock(const scoped_unblock& rhs);

        scoped_unblock&
        operator =(const scoped_unblock& rhs);

    public:
        ~scoped_unblock() NOTHROW
        {
            if (-1 == aug_blocksignals())
                aug_perrinfo("aug_blocksignals() failed");
        }

        scoped_unblock()
        {
            unblocksignals();
        }
    };
}

#endif // AUGSRVPP_SIGNAL_HPP
