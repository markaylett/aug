/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TYPES_HPP
#define AUGSYSPP_TYPES_HPP

#include "augsyspp/config.hpp"

#include "null.hpp"

namespace aug {

    // Helper functions for types that encapsulate c value types.

    template <typename T>
    const typename T::ctype&
    cref(const T& x)
    {
        return static_cast<const typename T::ctype&>(x);
    }

    template <typename T>
    typename T::ctype&
    cref(T& x)
    {
        return static_cast<typename T::ctype&>(x);
    }

    template <typename T>
    const typename T::ctype*
    cptr(const T& x)
    {
        return &static_cast<const typename T::ctype&>(x);
    }

    template <typename T>
    typename T::ctype*
    cptr(T& x)
    {
        return &static_cast<typename T::ctype&>(x);
    }

    class idref {
        int id_;
    public:
        idref(const null_&) AUG_NOTHROW
        : id_(-1)
        {
        }
        idref(int id) AUG_NOTHROW
        : id_(id)
        {
        }
        int
        get() const AUG_NOTHROW
        {
            return id_;
        }
    };

    inline bool
    operator ==(idref lhs, idref rhs)
    {
        return lhs.get() == rhs.get();
    }
    inline bool
    operator !=(idref lhs, idref rhs)
    {
        return lhs.get() != rhs.get();
    }
    inline bool
    operator >=(idref lhs, idref rhs)
    {
        return lhs.get() >= rhs.get();
    }
    inline bool
    operator >(idref lhs, idref rhs)
    {
        return lhs.get() > rhs.get();
    }
    inline bool
    operator <=(idref lhs, idref rhs)
    {
        return lhs.get() <= rhs.get();
    }
    inline bool
    operator <(idref lhs, idref rhs)
    {
        return lhs.get() < rhs.get();
    }

    typedef idref fdref;
}

inline bool
isnull(aug::idref ref)
{
    return -1 == ref.get();
}

#endif // AUGSYSPP_TYPES_HPP
