/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TYPES_HPP
#define AUGSYSPP_TYPES_HPP

#include "augsyspp/null.hpp"

namespace aug {

    class AUGSYSPP_API fdref {
        int fd_;
    public:
        fdref(const null_&) NOTHROW
        : fd_(-1)
        {
        }
        fdref(int fd) NOTHROW
        : fd_(fd)
        {
        }
        int
        get() const NOTHROW
        {
            return fd_;
        }
    };

    inline bool
    operator ==(fdref lhs, fdref rhs)
    {
        return lhs.get() == rhs.get();
    }
    inline bool
    operator !=(fdref lhs, fdref rhs)
    {
        return lhs.get() != rhs.get();
    }
    inline bool
    operator >=(fdref lhs, fdref rhs)
    {
        return lhs.get() >= rhs.get();
    }
    inline bool
    operator >(fdref lhs, fdref rhs)
    {
        return lhs.get() > rhs.get();
    }
    inline bool
    operator <=(fdref lhs, fdref rhs)
    {
        return lhs.get() <= rhs.get();
    }
    inline bool
    operator <(fdref lhs, fdref rhs)
    {
        return lhs.get() < rhs.get();
    }
    inline bool
    isnull(fdref ref)
    {
        return -1 == ref.get();
    }
}

#endif // AUGSYSPP_TYPES_HPP
