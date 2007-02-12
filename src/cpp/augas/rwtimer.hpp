/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_RWTIMER_HPP
#define AUGAS_RWTIMER_HPP

#include "augas/serv.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"

namespace augas {

    class rwtimer_base : public aug::timercb_base {

        virtual void
        do_setrwtimer(unsigned ms, unsigned flags) = 0;

        virtual void
        do_resetrwtimer(unsigned ms, unsigned flags) = 0;

        virtual void
        do_resetrwtimer(unsigned flags) = 0;

        virtual void
        do_cancelrwtimer(unsigned flags) = 0;

    public:
        ~rwtimer_base() AUG_NOTHROW;

        void
        setrwtimer(unsigned ms, unsigned flags)
        {
            do_setrwtimer(ms, flags);
        }
        void
        resetrwtimer(unsigned ms, unsigned flags)
        {
            do_resetrwtimer(ms, flags);
        }
        void
        resetrwtimer(unsigned flags)
        {
            do_resetrwtimer(flags);
        }
        void
        cancelrwtimer(unsigned flags)
        {
            do_cancelrwtimer(flags);
        }
    };

    typedef aug::smartptr<rwtimer_base> rwtimerptr;

    class rwtimer : public rwtimer_base {

        servptr serv_;
        const augas_object& sock_;
        aug::timer rdtimer_;
        aug::timer wrtimer_;

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned flags);

        void
        do_cancelrwtimer(unsigned flags);

    public:
        ~rwtimer() AUG_NOTHROW;

        rwtimer(const servptr& serv, const augas_object& sock,
                aug::timers& timers);
    };
}

#endif // AUGAS_RWTIMER_HPP
