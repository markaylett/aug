/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_EVENT_HPP
#define AUGUTILPP_EVENT_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "aubpp.hpp"

#include "augutil/event.h"

#include <utility> // std::pair<>

namespace aug {

    inline std::pair<int, aub::smartob<aub_object> >
    readevent(fdref ref)
    {
        aug_event event;
        verify(aug_readevent(ref.get(), &event));
        return std::make_pair(event.type_,
                              aub::object_attach(aub::obptr(event.ob_)));
    }

    inline const aug_event&
    writeevent(fdref ref, const aug_event& event)
    {
        return *verify(aug_writeevent(ref.get(), &event));
    }

    inline void
    writeevent(fdref ref, int type, aub::obref<aub_object> ob)
    {
        aug_event event = { type, ob.get() };
        writeevent(ref, event);
    }
}

#endif // AUGUTILPP_EVENT_HPP
