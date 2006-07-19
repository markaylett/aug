/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"
#include "augsys/errno.h"
#include "augsys/string.h" // aug_perror()

namespace aug {

    namespace detail {

        inline std::string
        join(const std::string& lhs, const std::string& rhs)
        {
            std::string s(lhs);
            s += ": ";
            s += rhs;
            return s;
        }
    }

    inline void
    init(struct aug_errinfo& errinfo)
    {
        if (-1 == aug_init(&errinfo))
            throw std::runtime_error(detail::join("aug_init() failed",
                                                  aug_strerror(errno)));
    }
    inline void
    term()
    {
        if (-1 == aug_term())
            throw std::runtime_error(detail::join("aug_term() failed",
                                                  aug_strerror(errno)));
    }
    inline void
    atexitinit(struct aug_errinfo& errinfo)
    {
        if (-1 == aug_atexitinit(&errinfo))
            throw std::runtime_error(detail::join("aug_atexitinit() failed",
                                                  aug_strerror(errno)));
    }
    inline void
    openfd(int fd, const struct aug_fddriver* driver)
    {
        if (-1 == aug_openfd(fd, driver))
            throwerrinfo("aug_openfd() failed");
    }
    inline void
    openfds(int fds[2], const struct aug_fddriver* driver)
    {
        if (-1 == aug_openfds(fds, driver))
            throwerrinfo("aug_openfds() failed");
    }
    inline void
    releasefd(int fd)
    {
        if (-1 == aug_releasefd(fd))
            throwerrinfo("aug_releasefd() failed");
    }
    inline void
    retainfd(int fd)
    {
        if (-1 == aug_retainfd(fd))
            throwerrinfo("aug_retainfd() failed");
    }
    inline void
    setfddriver(fdref ref, const struct aug_fddriver* driver)
    {
        if (-1 == aug_setfddriver(ref.get(), driver))
            throwerrinfo("aug_setfddriver() failed");
    }
    inline const struct aug_fddriver*
    fddriver(fdref ref)
    {
        const struct aug_fddriver* driver = aug_fddriver(ref.get());
        if (!driver)
            throwerrinfo("aug_fddriver() failed");
        return driver;
    }

    class scoped_init {

        scoped_init(const scoped_init& rhs);

        scoped_init&
        operator =(const scoped_init& rhs);

    public:
        ~scoped_init() NOTHROW
        {
            if (-1 == aug_term())
                aug_perror("aug_term() failed");
        }

        scoped_init(struct aug_errinfo& errinfo)
        {
            init(errinfo);
        }
    };
}

#endif // AUGSYSPP_BASE_HPP
