/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_EVENT_HPP
#define AUGUTILPP_EVENT_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augutil/event.h"

namespace aug {

    inline aug_event&
    readevent(fdref ref, aug_event& sig)
    {
        return *verify(aug_readevent(ref.get(), &sig));
    }

    inline const aug_event&
    writeevent(fdref ref, const aug_event& sig)
    {
        return *verify(aug_writeevent(ref.get(), &sig));
    }
}

#endif // AUGUTILPP_EVENT_HPP
