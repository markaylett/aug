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
#ifndef AUGUTILPP_TIMER_HPP
#define AUGUTILPP_TIMER_HPP

#include "augutilpp/object.hpp"

#include "augctxpp/exception.hpp"

#include "augabipp.hpp"

#include "augutil/list.h"
#include "augutil/timer.h"

#include "augctx/errno.h"

#include "augext/log.h"

namespace aug {

    template <void (*T)(aug::objectref, idref, unsigned&)>
    void
    timercb(aug_object* user, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            T(user, id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T, void (T::*U)(idref, unsigned&)>
    void
    timermemcb(aug_object* user, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            (obtop<T>(user)->*U)(id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    template <typename T>
    void
    timermemcb(aug_object* ob, int id, unsigned* ms) AUG_NOTHROW
    {
        try {
            obtop<T>(ob)->timercb(id, *ms);
        } AUG_SETERRINFOCATCH;
    }

    inline unsigned
    settimer(aug_timers_t timers, idref ref, unsigned ms, aug_timercb_t cb,
             aug::objectref ob)
    {
        return AUG_RESULT(verify(aug_settimer(timers, ref.get(), ms, cb,
                                              ob.get())));
    }

    inline unsigned
    settimer(aug_timers_t timers, idref ref, unsigned ms, aug_timercb_t cb,
             const null_&)
    {
        return AUG_RESULT(verify(aug_settimer(timers, ref.get(), ms, cb, 0)));
    }

    template <typename T>
    unsigned
    settimer(aug_timers_t timers, idref ref, unsigned ms, T& x)
    {
        aug::smartob<aug_boxptr> ob(createboxptr(getmpool(aug_tlx), &x, 0));
        return AUG_RESULT(verify(aug_settimer(timers, ref.get(), ms,
                                              timermemcb<T>, ob.base())));
    }

    template <typename T>
    unsigned
    settimer(aug_timers_t timers, idref ref, unsigned ms, std::auto_ptr<T>& x)
    {
        aug::smartob<aug_boxptr> ob(createboxptr(getmpool(aug_tlx), x));
        return AUG_RESULT(verify(aug_settimer(timers, ref.get(), ms,
                                              timermemcb<T>, ob.base())));
    }

    inline void
    resettimer(aug_timers_t timers, idref ref, unsigned ms = 0)
    {
        verify(aug_resettimer(timers, ref.get(), ms));
    }

    inline void
    canceltimer(aug_timers_t timers, idref ref)
    {
        verify(aug_canceltimer(timers, ref.get()));
    }

    inline bool
    expired(aug_timers_t timers, idref ref)
    {
        return aug_expired(timers, ref.get()) ? true : false;
    }

    inline void
    processexpired(aug_timers_t timers, bool force)
    {
        verify(aug_processexpired(timers, force ? AUG_TRUE : AUG_FALSE, 0));
    }

    inline void
    processexpired(aug_timers_t timers, bool force, aug_timeval& next)
    {
        verify(aug_processexpired(timers, force ? AUG_TRUE : AUG_FALSE,
                                  &next));
    }

    inline bool
    empty(aug_timers_t timers)
    {
        return aug_timersempty(timers) ? true : false;
    }

    class timers : public mpool_ops {

        aug_timers_t timers_;

        timers(const timers&);

        timers&
        operator =(const timers&);

    public:
        ~timers() AUG_NOTHROW
        {
            if (timers_)
                aug_destroytimers(timers_);
        }

        timers(const null_&) AUG_NOTHROW
           : timers_(0)
        {
        }

        explicit
        timers(mpoolref mpool)
            : timers_(aug_createtimers(mpool.get()))
        {
            verify(timers_);
        }

        void
        swap(timers& rhs) AUG_NOTHROW
        {
            std::swap(timers_, rhs.timers_);
        }

        operator aug_timers_t()
        {
            return timers_;
        }

        aug_timers_t
        get()
        {
            return timers_;
        }

        bool
        empty() const
        {
            return aug::empty(timers_);
        }
    };

    inline void
    swap(timers& lhs, timers& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    class timer : public mpool_ops {

        aug_timers_t timers_;
        idref ref_;

        timer(const timer& rhs);

        timer&
        operator =(const timer& rhs);

    public:
        ~timer() AUG_NOTHROW
        {
            if (idref(0) < ref_) {
                try {
                    canceltimer(timers_, ref_);
                } catch (const none_exception&) {
                }
            }
        }

        explicit
        timer(aug_timers_t timers, idref ref = null)
            : timers_(timers),
              ref_(ref)
        {
        }

        timer&
        operator =(const null_&)
        {
            try {
                cancel();
            } catch (const none_exception&) {
            }
            return *this;
        }

        void
        set(unsigned ms, aug_timercb_t cb, aug::objectref ob)
        {
            ref_ = settimer(timers_, ref_, ms, cb, ob);
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

        void
        reset(unsigned ms = 0)
        {
            if (null == ref_)
                throw none_exception();

            try {
                resettimer(timers_, ref_, ms);
            } catch (const none_exception&) {
                ref_ = null;
                throw;
            }
        }

        void
        cancel()
        {
            if (null == ref_)
                throw none_exception();

            idref ref(ref_);
            ref_ = null;
            canceltimer(timers_, ref);
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
isnull(aug_timers_t timers)
{
    return !timers;
}

inline bool
isnull(const aug::timer& timer)
{
    return null == timer.id();
}

#endif // AUGUTILPP_TIMER_HPP
