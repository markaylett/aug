/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_RWTIMER_HPP
#define AUGASPP_RWTIMER_HPP

#include "augaspp/session.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"
#include "augctxpp.hpp"

namespace aug {

    class rwtimer_base : public mpool_ops {

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

    class rwtimer : public rwtimer_base {

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
                timers& timers);
    };
}

#endif // AUGASPP_RWTIMER_HPP
