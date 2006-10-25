/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MUTEX_HPP
#define AUGSYSPP_MUTEX_HPP

#include "augsyspp/exception.hpp"

#include "augsys/mutex.h"

namespace aug {

    inline void
    lockmutex(aug_mutex_t mutex)
    {
        verify(aug_lockmutex(mutex));
    }

    inline void
    unlockmutex(aug_mutex_t mutex)
    {
        verify(aug_unlockmutex(mutex));
    }

    class mutex {

        aug_mutex_t mutex_;

        mutex(const mutex&);

        mutex&
        operator =(const mutex&);

    public:
        ~mutex() AUG_NOTHROW
        {
            if (-1 == aug_freemutex(mutex_))
                aug_perrinfo(0, "aug_freemutex() failed");
        }

        mutex()
            : mutex_(aug_createmutex())
        {
            verify(mutex_);
        }

        operator aug_mutex_t()
        {
            return mutex_;
        }

        aug_mutex_t
        get()
        {
            return mutex_;
        }
    };

    class scoped_lock {

        aug_mutex_t mutex_;

        scoped_lock(const scoped_lock& rhs);

        scoped_lock&
        operator =(const scoped_lock& rhs);

    public:
        ~scoped_lock() AUG_NOTHROW
        {
            if (-1 == aug_unlockmutex(mutex_))
                aug_perrinfo(0, "aug_unlockmutex() failed");
        }

        explicit
        scoped_lock(aug_mutex_t mutex)
            : mutex_(mutex)
        {
            lockmutex(mutex);
        }
    };

    class scoped_unlock {

        aug_mutex_t mutex_;

        scoped_unlock(const scoped_unlock& rhs);

        scoped_unlock&
        operator =(const scoped_unlock& rhs);

    public:
        ~scoped_unlock() AUG_NOTHROW
        {
            if (-1 == aug_lockmutex(mutex_))
                aug_perrinfo(0, "aug_lockmutex() failed");
        }

        explicit
        scoped_unlock(aug_mutex_t mutex)
            : mutex_(mutex)
        {
            unlockmutex(mutex);
        }
    };
}

#endif // AUGSYSPP_MUTEX_HPP
