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
#ifndef AUGASPP_RWTIMER_HPP
#define AUGASPP_RWTIMER_HPP

#include "augaspp/session.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"
#include "augctxpp.hpp"

namespace aug {

    class rwtimer_base {

        virtual void
        do_timercb(idref id, unsigned& ms) = 0;

        virtual void
        do_setrwtimer(unsigned ms, unsigned flags) = 0;

        virtual bool
        do_resetrwtimer(unsigned ms, unsigned flags) = 0;

        virtual bool
        do_resetrwtimer(unsigned flags) = 0;

        virtual bool
        do_cancelrwtimer(unsigned flags) = 0;

    public:
        virtual
        ~rwtimer_base() AUG_NOTHROW;

        void
        timercb(idref id, unsigned& ms)
        {
            do_timercb(id, ms);
        }
        void
        setrwtimer(unsigned ms, unsigned flags)
        {
            do_setrwtimer(ms, flags);
        }
        bool
        resetrwtimer(unsigned ms, unsigned flags)
        {
            return do_resetrwtimer(ms, flags);
        }
        bool
        resetrwtimer(unsigned flags)
        {
            return do_resetrwtimer(flags);
        }
        bool
        cancelrwtimer(unsigned flags)
        {
            return do_cancelrwtimer(flags);
        }
    };

    typedef smartptr<rwtimer_base> rwtimerptr;

    class rwtimer : public rwtimer_base, public mpool_ops {

        sessionptr session_;
        const mod_handle& sock_;
        timer rdtimer_;
        timer wrtimer_;

        void
        do_timercb(idref id, unsigned& ms);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned flags);

        bool
        do_cancelrwtimer(unsigned flags);

    public:
        ~rwtimer() AUG_NOTHROW;

        rwtimer(const sessionptr& session, const mod_handle& sock,
                aug_timers_t timers);
    };
}

#endif // AUGASPP_RWTIMER_HPP
