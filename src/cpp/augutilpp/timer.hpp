/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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

#include <memory> // auto_ptr<>

namespace aug {

    template <void (*T)(const aug_var&, int, unsigned&, aug_timers&)>
    void
    timercb(const aug_var* var, int id, unsigned* ms,
            aug_timers* timers) AUG_NOTHROW
    {
        try {
            T(*var, id, *ms, *timers);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T, void (T::*U)(int, unsigned&, aug_timers&)>
    void
    timermemcb(const aug_var* var, int id, unsigned* ms,
               aug_timers* timers) AUG_NOTHROW
    {
        try {
            (static_cast<T*>(var->arg_)->*U)(id, *ms, *timers);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T>
    void
    timermemcb(const aug_var* var, int id, unsigned* ms,
               aug_timers* timers) AUG_NOTHROW
    {
        try {
            static_cast<T*>(var->arg_)->timercb(id, *ms, *timers);
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
             const aug_var& var)
    {
        return verify(aug_settimer(&timers, ref.get(), ms, cb, &var));
    }

    inline int
    settimer(aug_timers& timers, idref ref, unsigned ms, aug_timercb_t cb,
             const null_&)
    {
        return verify(aug_settimer(&timers, ref.get(), ms, cb, 0));
    }

    template <typename T>
    int
    settimer(aug_timers& timers, idref ref, unsigned ms, T& x)
    {
        aug_var var = { 0, &x };
        return verify(aug_settimer(&timers, ref.get(), ms,
                                   timermemcb<T>, &var));
    }

    template <typename T>
    int
    settimer(aug_timers& timers, idref ref, unsigned ms, std::auto_ptr<T>& x)
    {
        aug_var var;
        int id(verify(aug_settimer(&timers, ref.get(), ms, timermemcb<T>,
                                   &bindvar<deletearg<T> >(var, *x))));
        x.release();
        return id;
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
        set(unsigned ms, aug_timercb_t cb, const aug_var& var)
        {
            ref_ = settimer(timers_, ref_, ms, cb, var);
        }

        void
        set(unsigned ms, aug_timercb_t cb, const null_&)
        {
            ref_ = settimer(timers_, ref_, ms, cb, null);
        }

        template <typename T>
        void
        set(unsigned ms, T& x)
        {
            ref_ = settimer(timers_, ref_, ms, x);
        }

        template <typename T>
        void
        set(unsigned ms, std::auto_ptr<T>& x)
        {
            ref_ = settimer(timers_, ref_, ms, x);
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
