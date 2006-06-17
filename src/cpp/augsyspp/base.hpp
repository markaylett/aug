/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_BASE_HPP
#define AUGSYSPP_BASE_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/types.hpp"

#include "augsys/base.h"

namespace aug {

    inline void
    init()
    {
        if (-1 == aug_init())
            error("aug_init() failed");
    }
    inline void
    term()
    {
        if (-1 == aug_term())
            error("aug_term() failed");
    }
    inline void
    openfd(int fd, int type)
    {
        if (-1 == aug_openfd(fd, type))
            error("aug_openfd() failed");
    }
    inline void
    releasefd(int fd)
    {
        if (-1 == aug_releasefd(fd))
            error("aug_releasefd() failed");
    }
    inline void
    retainfd(int fd)
    {
        if (-1 == aug_retainfd(fd))
            error("aug_retainfd() failed");
    }
    inline aug_fdhook_t
    setfdhook(fdref ref, aug_fdhook_t hook, void* data)
    {
        if (-1 == aug_setfdhook(ref.get(), &hook, data))
            error("aug_setfdhook() failed");
        return hook;
    }
    inline void
    setfdtype(fdref ref, int type)
    {
        if (-1 == aug_setfdtype(ref.get(), type))
            error("aug_setfdtype() failed");
    }
    inline void
    setfddata(fdref ref, void* data)
    {
        if (-1 == aug_setfddata(ref.get(), data))
            error("aug_setfddata() failed");
    }
    inline int
    fdtype(fdref ref)
    {
        int ret;
        if (-1 == (ret = aug_fdtype(ref.get())))
            error("aug_fdtype() failed");
        return ret;
    }
    inline void*
    fddata(fdref ref)
    {
        void* ret;
        if (-1 == aug_fddata(ref.get(), &ret))
            error("aug_fddata() failed");
        return ret;
    }

    class AUGSYSPP_API initialiser {

        initialiser(const initialiser& rhs);

        initialiser&
        operator =(const initialiser& rhs);

    public:
        ~initialiser() NOTHROW;

        initialiser();
    };
}

#endif // AUGSYSPP_BASE_HPP
