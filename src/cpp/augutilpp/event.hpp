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
        return *verify(aug_readevent(ref.get(), &sig));
    }

    inline const struct aug_event&
    writeevent(fdref ref, const struct aug_event& sig)
    {
        return *verify(aug_writeevent(ref.get(), &sig));
    }
}

#endif // AUGUTILPP_EVENT_HPP
