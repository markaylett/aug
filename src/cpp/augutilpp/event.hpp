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

#include "augabipp.hpp"

#include "augutil/event.h"

#include <utility> // std::pair<>

namespace aug {

    inline aug_event&
    setsigevent(aug_event& event, int sig)
    {
        return *aug_setsigevent(&event, sig);
    }

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
