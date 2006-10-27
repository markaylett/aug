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

#include <stdexcept>

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
    init(aug_errinfo& errinfo)
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
    atexitinit(aug_errinfo& errinfo)
    {
        if (-1 == aug_atexitinit(&errinfo))
            throw std::runtime_error(detail::join("aug_atexitinit() failed",
                                                  aug_strerror(errno)));
    }
    inline void
    openfd(int fd, const aug_driver* driver)
    {
        verify(aug_openfd(fd, driver));
    }
    inline void
    openfds(int fds[2], const aug_driver* driver)
    {
        verify(aug_openfds(fds, driver));
    }
    inline void
    releasefd(int fd)
    {
        verify(aug_releasefd(fd));
    }
    inline void
    retainfd(int fd)
    {
        verify(aug_retainfd(fd));
    }
    inline void
    setdriver(fdref ref, const aug_driver* driver)
    {
        verify(aug_setdriver(ref.get(), driver));
    }
    inline const aug_driver*
    getdriver(fdref ref)
    {
        return verify(aug_getdriver(ref.get()));
    }

    class scoped_init {

        scoped_init(const scoped_init& rhs);

        scoped_init&
        operator =(const scoped_init& rhs);

    public:
        ~scoped_init() AUG_NOTHROW
        {
            if (-1 == aug_term())
                aug_perror("aug_term() failed");
        }

        scoped_init(aug_errinfo& errinfo)
        {
            init(errinfo);
        }
    };
}

#endif // AUGSYSPP_BASE_HPP
