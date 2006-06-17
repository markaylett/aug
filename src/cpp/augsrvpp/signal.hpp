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
            throwerror("aug_signalhandler() failed");
    }

    inline void
    blocksignals()
    {
        if (-1 == aug_blocksignals())
            throwerror("aug_blocksignals() failed");
    }

    inline void
    unblocksignals()
    {
        if (-1 == aug_unblocksignals())
            throwerror("aug_unblocksignals() failed");
    }
}

#endif // AUGSRVPP_SIGNAL_HPP
