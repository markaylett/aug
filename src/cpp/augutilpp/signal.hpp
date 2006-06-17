/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_SIGNAL_HPP
#define AUGUTILPP_SIGNAL_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augutil/signal.h"

namespace aug {

    inline aug_signal_t
    readsignal(fdref ref)
    {
        aug_signal_t sig;
        if (-1 == aug_readsignal(ref.get(), &sig))
            error("aug_readsignal() failed");
        return sig;
    }

    inline void
    writesignal(fdref ref, aug_signal_t sig)
    {
        if (-1 == aug_writesignal(ref.get(), sig))
            error("aug_writesignal() failed");
    }
}

#endif // AUGUTILPP_SIGNAL_HPP
