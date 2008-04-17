/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augctx/base.h"

#include <stdexcept>

namespace aug {

    inline void
    init(aug_errinfo& errinfo)
    {
        if (aug_init(&errinfo) < 0)
            throw std::runtime_error("aug_init() failed");
    }
    inline void
    term()
    {
        if (aug_term() < 0)
            throw std::runtime_error("aug_term() failed");
    }
    inline void
    start()
    {
        if (aug_start() < 0)
            throw std::runtime_error("aug_start() failed");
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
            aug_term();
        }
        scoped_init()
        {
            init();
        }
    };
}

#endif // AUGSYSPP_BASE_HPP
