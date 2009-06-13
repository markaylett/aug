/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGUTILPP_EVENT_HPP
#define AUGUTILPP_EVENT_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/types.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augabipp.hpp"

#include "augutil/event.h"

#include <utility> // std::pair<>

namespace aug {

    inline std::pair<int, objectptr>
    readevent(aug_events_t events)
    {
        aug_event event;
        verify(aug_readevent(events, &event));
        return std::make_pair(event.type_,
                              aug::object_attach(aug::obptr(event.ob_)));
    }

    inline void
    writeevent(aug_events_t events, const aug_event& event)
    {
        verify(aug_writeevent(events, &event));
    }

    inline void
    writeevent(aug_events_t events, int type, objectref ob)
    {
        aug_event event = { type, ob.get() };
        writeevent(events, event);
    }

    inline mdref
    eventsmd(aug_events_t events)
    {
        return aug_eventsmd(events);
    }

    class events : public mpool_ops {

        aug_events_t events_;

        events(const events&);

        events&
        operator =(const events&);

    public:
        ~events() AUG_NOTHROW
        {
            if (events_)
                aug_destroyevents(events_);
        }

        events(const null_&) AUG_NOTHROW
           : events_(0)
        {
        }

        explicit
        events(mpoolref mpool)
            : events_(aug_createevents(mpool.get()))
        {
            verify(events_);
        }

        void
        swap(events& rhs) AUG_NOTHROW
        {
            std::swap(events_, rhs.events_);
        }

        operator aug_events_t()
        {
            return events_;
        }

        aug_events_t
        get()
        {
            return events_;
        }
    };

    inline void
    swap(events& lhs, events& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline aug_event&
    sigtoevent(int sig, aug_event& event)
    {
        return *aug_sigtoevent(sig, &event);
    }
}

inline bool
isnull(aug_events_t events)
{
    return !events;
}

#endif // AUGUTILPP_EVENT_HPP
