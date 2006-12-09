/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_MUTEX_HPP
#define AUGSYSPP_MUTEX_HPP

#include "augsyspp/exception.hpp"

#include "augsys/mutex.h"

namespace aug {

    class mutex {

        aug_mutex_t mutex_;

        mutex(const mutex&);

        mutex&
        operator =(const mutex&);

    public:
        ~mutex() AUG_NOTHROW
        {
            if (-1 == aug_freemutex(mutex_))
                perrinfo("aug_freemutex() failed");
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
}

#endif // AUGSYSPP_MUTEX_HPP
