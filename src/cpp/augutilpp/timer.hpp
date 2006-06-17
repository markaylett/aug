/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_TIMER_HPP
#define AUGUTILPP_TIMER_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/timer.h"

namespace aug {

    class AUGUTILPP_API timers {

        friend class timer;

        struct aug_timers timers_;

        timers(const timers&);

        timers&
        operator =(const timers&);

    public:
        ~timers() NOTHROW;

        timers();

        operator struct aug_timers&()
        {
            return timers_;
        }
        struct aug_timers&
        get()
        {
            return timers_;
        }

        bool
        empty() const;
    };

    inline void
    processtimers(struct aug_timers& timers, bool force)
    {
        if (-1 == aug_processtimers(&timers, force ? 1 : 0, 0))
            error("aug_processtimers() failed");
    }

    inline void
    processtimers(struct aug_timers& timers, bool force, struct timeval& next)
    {
        if (-1 == aug_processtimers(&timers, force ? 1 : 0, &next))
            error("aug_processtimers() failed");
    }

    class AUGUTILPP_API expire_base {

        virtual void
        do_expire(int id) = 0;

    public:
        virtual
        ~expire_base() NOTHROW;

        void
        expire(int id)
        {
            do_expire(id);
        }
    };

    class AUGUTILPP_API timer {

        struct aug_timers& timers_;
        expire_base& action_;
        int id_;
        bool pending_;

        timer(const timer&);

        timer&
        operator =(const timer&);

        static void
        expire_(void* arg, int id);

    public:
        ~timer() NOTHROW;

        timer(timers& timers, expire_base& action)
            : timers_(timers.timers_),
              action_(action),
              id_(-1),
              pending_(false)
        {
        }

        timer(timers& timers, unsigned int ms, expire_base& action)
            : timers_(timers.timers_),
              action_(action),
              id_(-1),
              pending_(false)
        {
            reset(ms);
        }

        void
        cancel();

        void
        reset(unsigned int ms);

        int
        get() const
        {
            return id_;
        }

        bool
        pending() const
        {
            return -1 != id_ && pending_;
        }
    };
}

#endif // AUGUTILPP_TIMER_HPP
