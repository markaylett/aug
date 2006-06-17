/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"
#include "augsys/string.h" // aug_perror()

namespace aug {

    inline void
    init(struct aug_errinfo& errinfo)
    {
        if (-1 == aug_init(&errinfo))
            throwerror("aug_init() failed");
    }
    inline void
    term()
    {
        if (-1 == aug_term())
            throwerror("aug_term() failed");
    }
    inline void
    openfd(int fd, const struct aug_fddriver* driver)
    {
        if (-1 == aug_openfd(fd, driver))
            throwerror("aug_openfd() failed");
    }
    inline void
    releasefd(int fd)
    {
        if (-1 == aug_releasefd(fd))
            throwerror("aug_releasefd() failed");
    }
    inline void
    retainfd(int fd)
    {
        if (-1 == aug_retainfd(fd))
            throwerror("aug_retainfd() failed");
    }
    inline void
    setfddriver(fdref ref, const struct aug_fddriver* driver)
    {
        if (-1 == aug_setfddriver(ref.get(), driver))
            throwerror("aug_setfddriver() failed");
    }
    inline const struct aug_fddriver*
    fddriver(fdref ref)
    {
        const struct aug_fddriver* driver = aug_fddriver(ref.get());
        if (!driver)
            throwerror("aug_fddriver() failed");
        return driver;
    }

    class AUGSYSPP_API initialiser {

        initialiser(const initialiser& rhs);

        initialiser&
        operator =(const initialiser& rhs);

    public:
        ~initialiser() NOTHROW
        {
            if (-1 == aug_term())
                aug_perror("aug_term() failed");
        }

        initialiser(struct aug_errinfo& errinfo)
        {
            init(errinfo);
        }
    };
}

#endif // AUGSYSPP_BASE_HPP
