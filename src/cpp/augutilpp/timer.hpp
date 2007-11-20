/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_TIMER_HPP
#define AUGUTILPP_TIMER_HPP

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/utility.hpp"

#include "augobjpp.hpp"

#include "augutil/list.h"
#include "augutil/timer.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    template <void (*T)(aug_object_t, int, unsigned&)>
    void
    timercb(aug_object_t user, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            T(user, id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T, void (T::*U)(int, unsigned&)>
    void
    timermemcb(aug_object_t user, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            (objtoptr<T*>(user)->*U)(id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T>
    void
    timermemcb(aug_object_t user, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            objtoptr<T*>(user)->timercb(id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    class timers {
    public:
        typedef aug_timers ctype;
    private:

        friend class timer;

        aug_timers timers_;

        timers(const timers&);

        timers&
        operator =(const timers&);

    public:
        ~timers() AUG_NOTHROW
        {
            if (-1 == aug_destroytimers(&timers_))
                perrinfo("aug_destroytimers() failed");
        }

        timers()
        {
            AUG_INIT(&timers_);
        }

        operator aug_timers&()
        {
            return timers_;
        }

        operator const aug_timers&() const
        {
            return timers_;
        }

        bool
        empty() const
        {
            return AUG_EMPTY(&timers_);
        }
    };

    inline int
    settimer(aug_timers& timers, idref ref, unsigned ms, aug_timercb_t cb,
             aug_object_t user)
    {
        return verify(aug_settimer(&timers, ref.get(), ms, cb, user));
    }

    inline int
    settimer(aug_timers& timers, idref ref, unsigned ms, aug_timercb_t cb,
             const null_&)
    {
        return verify(aug_settimer(&timers, ref.get(), ms, cb, 0));
    }

    inline bool
    resettimer(aug_timers& timers, idref ref, unsigned ms = 0)
    {
        return AUG_RETNONE == verify(aug_resettimer(&timers, ref.get(), ms))
            ? false : true;
    }

    inline bool
    canceltimer(aug_timers& timers, idref ref)
    {
        return AUG_RETNONE == aug_canceltimer(&timers, ref.get())
            ? false : true;
    }

    inline bool
    expired(aug_timers& timers, idref ref)
    {
        return aug_expired(&timers, ref.get()) ? true : false;
    }

    inline void
    foreachexpired(aug_timers& timers, bool force)
    {
        verify(aug_foreachexpired(&timers, force ? 1 : 0, 0));
    }

    inline void
    foreachexpired(aug_timers& timers, bool force, timeval& next)
    {
        verify(aug_foreachexpired(&timers, force ? 1 : 0, &next));
    }

    class timer {

        aug_timers& timers_;
        idref ref_;

        timer(const timer& rhs);

        timer&
        operator =(const timer& rhs);

    public:
        ~timer() AUG_NOTHROW
        {
            if (0 < ref_)
                canceltimer(timers_, ref_);
        }

        explicit
        timer(aug_timers& timers, idref ref = null)
            : timers_(timers),
              ref_(ref)
        {
        }

        timer&
        operator =(const null_&)
        {
            cancel();
            return *this;
        }

        void
        set(unsigned ms, aug_timercb_t cb, aug_object_t user)
        {
            ref_ = settimer(timers_, ref_, ms, cb, user);
        }

        void
        set(unsigned ms, aug_timercb_t cb, const null_&)
        {
            ref_ = settimer(timers_, ref_, ms, cb, null);
        }

        bool
        reset(unsigned ms = 0)
        {
            if (null == ref_)
                return false;

            if (!resettimer(timers_, ref_, ms)) {
                ref_ = null;
                return false;
            }

            return true;
        }

        bool
        cancel()
        {
            if (null == ref_)
                return false;

            idref ref(ref_);
            ref_ = null;
            return canceltimer(timers_, ref);
        }

        bool
        expired() const
        {
            return aug::expired(timers_, ref_);
        }

        idref
        id() const
        {
            return ref_;
        }
    };
}

inline bool
isnull(const aug::timer& t)
{
    return null == t.id();
}

#endif // AUGUTILPP_TIMER_HPP
