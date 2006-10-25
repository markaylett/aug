/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_TIMER_HPP
#define AUGUTILPP_TIMER_HPP

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/list.h"
#include "augutil/timer.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    class timers {
    public:
        typedef struct aug_timers ctype;
    private:

        friend class timer;

        struct aug_timers timers_;

        timers(const timers&);

        timers&
        operator =(const timers&);

    public:
        ~timers() AUG_NOTHROW
        {
            if (-1 == aug_freetimers(&timers_))
                aug_perrinfo(0, "aug_freetimers() failed");
        }

        timers()
        {
            AUG_INIT(&timers_);
        }

        operator struct aug_timers&()
        {
            return timers_;
        }

        operator const struct aug_timers&() const
        {
            return timers_;
        }

        bool
        empty() const
        {
            return AUG_EMPTY(&timers_);
        }
    };

    class timercb_base {

        virtual void
        do_callback(idref ref, unsigned& ms, struct aug_timers& timers) = 0;

    public:
        virtual
        ~timercb_base() AUG_NOTHROW
        {
        }

        void
        callback(idref ref, unsigned& ms, struct aug_timers& timers)
        {
            return do_callback(ref.get(), ms,timers);
        }

        void
        operator ()(idref ref, unsigned& ms, struct aug_timers& timers)
        {
            return do_callback(ref.get(), ms, timers);
        }
    };

    namespace detail {

        inline void
        timercb(const struct aug_var* arg, int id, unsigned* ms,
                struct aug_timers* timers)
        {
            try {
                timercb_base* ptr = static_cast<
                    timercb_base*>(aug_getvarp(arg));
                ptr->callback(id, *ms, *timers);
            } AUG_SETERRINFOCATCH;
        }
    }

    inline int
    settimer(struct aug_timers& timers, idref ref, unsigned ms,
             timercb_base& cb)
    {
        var v(&cb);
        return verify(aug_settimer(&timers, ref.get(), ms, detail::timercb,
                                   cptr(v)));
    }

    inline bool
    resettimer(struct aug_timers& timers, idref ref, unsigned ms = 0)
    {
        return AUG_RETNONE == verify(aug_resettimer(&timers, ref.get(), ms))
            ? false : true;
    }

    inline bool
    canceltimer(struct aug_timers& timers, idref ref)
    {
        return aug_canceltimer(&timers, ref.get()) ? true : false;
    }

    inline bool
    expired(struct aug_timers& timers, idref ref)
    {
        return aug_expired(&timers, ref.get()) ? true : false;
    }

    inline void
    processtimers(struct aug_timers& timers, bool force)
    {
        verify(aug_processtimers(&timers, force ? 1 : 0, 0));
    }

    inline void
    processtimers(struct aug_timers& timers, bool force, struct timeval& next)
    {
        verify(aug_processtimers(&timers, force ? 1 : 0, &next));
    }

    class timer {

        struct aug_timers& timers_;
        idref ref_;

        timer(const timer& rhs);

        timer&
        operator =(const timer& rhs);

    public:
        ~timer() AUG_NOTHROW
        {
            if (null != ref_)
                canceltimer(timers_, ref_);
        }

        timer(struct aug_timers& timers, idref ref)
            : timers_(timers),
              ref_(ref)
        {
        }

        timer&
        operator =(const null_&)
        {
            cancel();
            ref_ = null;
            return *this;
        }

        void
        set(unsigned ms, timercb_base& cb)
        {
            ref_ = settimer(timers_, ref_, ms, cb);
        }

        bool
        reset(unsigned ms = 0)
        {
            return resettimer(timers_, ref_, ms);
        }

        bool
        cancel()
        {
            return canceltimer(timers_, ref_);
        }

        bool
        expired() const
        {
            return aug::expired(timers_, ref_);
        }

        operator idref() const
        {
            return ref_;
        }

        int
        get() const
        {
            return ref_.get();
        }
    };
}

#endif // AUGUTILPP_TIMER_HPP
