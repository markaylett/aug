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
    sigactions(void (*handler)(int))
    {
        if (-1 == aug_sigactions(handler))
            error("aug_sigactions() failed");
    }

    inline void
    sigblock()
    {
        if (-1 == aug_sigblock())
            error("aug_sigblock() failed");
    }

    inline void
    sigunblock()
    {
        if (-1 == aug_sigunblock())
            error("aug_sigunblock() failed");
    }

    inline aug_sig_t
    readsig()
    {
        aug_sig_t sig;
        if (-1 == aug_readsig(&sig))
            error("aug_readsig() failed");
        return sig;
    }

    inline void
    writesig(aug_sig_t sig)
    {
        if (-1 == aug_writesig(sig))
            error("aug_writesig() failed");
    }
}

#endif // AUGSRVPP_SIGNAL_HPP
