/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_LOCK_HPP
#define AUGSYSPP_LOCK_HPP

#include "augsyspp/exception.hpp"

#include "augsys/lock.h"
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

    class scoped_lock {

        aug_mutex_t mutex_;

        scoped_lock(const scoped_lock& rhs);

        scoped_lock&
        operator =(const scoped_lock& rhs);

    public:
        ~scoped_lock() AUG_NOTHROW
        {
            if (!mutex_)
                aug_unlock();
            else if (-1 == aug_unlockmutex(mutex_))
                perrinfo("aug_unlockmutex() failed");
        }
        scoped_lock()
            : mutex_(0)
        {
            aug_lock();
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
            if (!mutex_)
                aug_lock();
            if (-1 == aug_lockmutex(mutex_))
                perrinfo("aug_lockmutex() failed");
        }
        scoped_unlock()
            : mutex_(0)
        {
            aug_unlock();
        }
        explicit
        scoped_unlock(aug_mutex_t mutex)
            : mutex_(mutex)
        {
            unlockmutex(mutex);
        }
    };
}

#endif // AUGSYSPP_LOCK_HPP
