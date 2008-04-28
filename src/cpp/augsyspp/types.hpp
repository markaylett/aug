/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TYPES_HPP
#define AUGSYSPP_TYPES_HPP

#include "augsyspp/config.hpp"

#include "augnullpp.hpp"

#include "augsys/config.h"
#include "augsys/types.h"

#include "augtypes.h"

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

    template <typename traitsT>
    class basic_ref {
        typename traitsT::ref ref_;
    public:
        basic_ref(const null_&) AUG_NOTHROW
        : ref_(traitsT::bad())
        {
        }
        basic_ref(typename traitsT::ref ref) AUG_NOTHROW
        : ref_(ref)
        {
        }
        typename traitsT::ref
        get() const AUG_NOTHROW
        {
            return ref_;
        }
    };

    template <typename traitsT>
    bool
    operator ==(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return 0 == traitsT::compare(lhs.get(), rhs.get());
    }
    template <typename traitsT>
    bool
    operator !=(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return 0 != traitsT::compare(lhs.get(), rhs.get());
    }
    template <typename traitsT>
    bool
    operator >=(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return 0 <= traitsT::compare(lhs.get(), rhs.get());
    }
    template <typename traitsT>
    bool
    operator >(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return 0 < traitsT::compare(lhs.get(), rhs.get());
    }
    template <typename traitsT>
    bool
    operator <=(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return traitsT::compare(lhs.get(), rhs.get()) <= 0;
    }
    template <typename traitsT>
    bool
    operator <(basic_ref<traitsT> lhs, basic_ref<traitsT> rhs)
    {
        return traitsT::compare(lhs.get(), rhs.get()) < 0;
    }

    struct id_traits {
        typedef int ref;
        static int
        bad() AUG_NOTHROW
        {
            return -1;
        }
        static int
        compare(int lhs, int rhs) AUG_NOTHROW
        {
            if (lhs < rhs)
                return -1;
            if (rhs < lhs)
                return 1;
            return 0;
        }
    };

    struct fd_traits {
        typedef aug_fd ref;
        static aug_fd
        bad() AUG_NOTHROW
        {
            return AUG_BADFD;
        }
        static int
        compare(aug_fd lhs, aug_fd rhs) AUG_NOTHROW
        {
            if (lhs < rhs)
                return -1;
            if (rhs < lhs)
                return 1;
            return 0;
        }
    };

    struct sd_traits {
        typedef aug_sd ref;
        static aug_sd
        bad() AUG_NOTHROW
        {
            return AUG_BADSD;
        }
        static int
        compare(aug_sd lhs, aug_sd rhs) AUG_NOTHROW
        {
            if (lhs < rhs)
                return -1;
            if (rhs < lhs)
                return 1;
            return 0;
        }
    };

    typedef basic_ref<id_traits> idref;
    typedef basic_ref<fd_traits> fdref;
    typedef basic_ref<sd_traits> sdref;
    typedef basic_ref<sd_traits> mdref;
}

template <typename T>
bool
isnull(aug::basic_ref<T> ref)
{
    return T::bad() == ref.get();
}

#endif // AUGSYSPP_TYPES_HPP
