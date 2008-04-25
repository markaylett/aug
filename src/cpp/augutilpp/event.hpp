/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_EVENT_HPP
#define AUGUTILPP_EVENT_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augabipp.hpp"

#include "augutil/event.h"

#include <utility> // std::pair<>

namespace aug {

    inline std::pair<int, aug::smartob<aug_object> >
    readevent(mdref ref)
    {
        aug_event event;
        verify(aug_readevent(ref.get(), &event));
        return std::make_pair(event.type_,
                              aug::object_attach(aug::obptr(event.ob_)));
    }

    inline const aug_event&
    writeevent(mdref ref, const aug_event& event)
    {
        return *verify(aug_writeevent(ref.get(), &event));
    }

    inline void
    writeevent(mdref ref, int type, aug::obref<aug_object> ob)
    {
        aug_event event = { type, ob.get() };
        writeevent(ref, event);
    }
}

#endif // AUGUTILPP_EVENT_HPP
