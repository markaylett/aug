/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_SIGNAL_HPP
#define AUGSRVPP_SIGNAL_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/signal.h"

namespace aug {

    void
    sigactions(void (*handler)(int))
    {
        if (-1 == aug_sigactions(handler))
            error("aug_sigactions() failed");
    }

    void
    sigblock()
    {
        if (-1 == aug_sigblock())
            error("aug_sigblock() failed");
    }

    void
    sigunblock()
    {
        if (-1 == aug_sigunblock())
            error("aug_sigunblock() failed");
    }

    enum aug_signal
    readsig()
    {
        enum aug_signal sig;
        if (-1 == aug_readsig(&sig))
            error("aug_readsig() failed");
        return sig;
    }

    void
    writesig(enum aug_signal sig)
    {
        if (-1 == aug_writesig(sig))
            error("aug_writesig() failed");
    }
}

#endif // AUGSRVPP_SIGNAL_HPP
