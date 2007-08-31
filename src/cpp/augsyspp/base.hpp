/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"
#include "augsys/errno.h"
#include "augsys/string.h" // aug_strerror()

#include <iostream>
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
    openfd(int fd, const aug_fdtype* fdtype)
    {
        verify(aug_openfd(fd, fdtype));
    }
    inline void
    openfds(int fds[2], const aug_fdtype* fdtype)
    {
        verify(aug_openfds(fds, fdtype));
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
    inline const aug_fdtype&
    setfdtype(fdref ref, const aug_fdtype& fdtype)
    {
        return *verify(aug_setfdtype(ref.get(), &fdtype));
    }
    inline const aug_fdtype&
    setfdtype(fdref ref)
    {
        return *verify(aug_setfdtype(ref.get(), 0));
    }
    inline const aug_fdtype&
    getfdtype(fdref ref)
    {
        return *verify(aug_getfdtype(ref.get()));
    }
    inline aug_fdtype&
    extfdtype(aug_fdtype& derived, const struct aug_fdtype& base)
    {
        return *aug_extfdtype(&derived, &base);
    }
    inline aug_fdtype&
    extfdtype(aug_fdtype& derived)
    {
        return *aug_extfdtype(&derived, 0);
    }

    class scoped_init {

        scoped_init(const scoped_init& rhs);

        scoped_init&
        operator =(const scoped_init& rhs);

    public:
        ~scoped_init() AUG_NOTHROW
        {
            if (-1 == aug_term())
                std::cerr << "aug_term() failed\n";
        }
        explicit
        scoped_init(aug_errinfo& errinfo)
        {
            init(errinfo);
        }
    };
}

#endif // AUGSYSPP_BASE_HPP
