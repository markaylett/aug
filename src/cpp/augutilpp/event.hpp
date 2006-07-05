/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_EVENT_HPP
#define AUGUTILPP_EVENT_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augutil/event.h"

namespace aug {

    inline struct aug_event&
    readevent(fdref ref, struct aug_event& sig)
    {
        if (!aug_readevent(ref.get(), &sig))
            throwerrinfo("aug_readevent() failed");
        return sig;
    }

    inline const struct aug_event&
    writeevent(fdref ref, const struct aug_event& sig)
    {
        if (!aug_writeevent(ref.get(), &sig))
            throwerrinfo("aug_writeevent() failed");
        return sig;
    }
}

#endif // AUGUTILPP_EVENT_HPP
