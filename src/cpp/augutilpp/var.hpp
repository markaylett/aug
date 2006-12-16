/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_VAR_HPP
#define AUGUTILPP_VAR_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/types.hpp"

#include "augutil/var.h"

namespace aug {

    namespace detail {

        template <typename T>
        struct var_traits;

        template <>
        struct var_traits<long> {
            static long
            get(const aug_var& v)
            {
                return aug_getvarl(&v);
            }
        };

        template <>
        struct var_traits<void*> {
            static void*
            get(const aug_var& v)
            {
                return aug_getvarp(&v);
            }
        };
    }

    inline const aug_var&
    freevar(const aug_var& v)
    {
        return *aug_freevar(&v);
    }

    inline aug_var&
    clearvar(aug_var& v, void (*fn)(void) = 0)
    {
        return *aug_clearvar(&v, fn);
    }

    inline aug_var&
    setvar(aug_var& v, const aug_var& w)
    {
        return *aug_setvar(&v, &w);
    }

    inline aug_var&
    setvar(aug_var& v, long l, void (*fn)(long) = 0)
    {
        return *aug_setvarl(&v, l, fn);
    }

    inline aug_var&
    setvar(aug_var& v, void* p, void (*fn)(void*) = 0)
    {
        return *aug_setvarp(&v, p, fn);
    }

    template <typename T>
    T
    getvar(const aug_var& v)
    {
        return detail::var_traits<T>::get(v);
    }

    inline aug_vartype
    type(const aug_var& v)
    {
        return v.type_;
    }

    class var {
    public:
        typedef aug_var ctype;
    private:
        aug_var var_;

        var(const var&);

        var&
        operator =(const var&);

    public:
        explicit
        var(const null_&, void (*fn)(void) = 0) AUG_NOTHROW
        {
            clearvar(var_, fn);
        }

        explicit
        var(long l, void (*fn)(long) = 0) AUG_NOTHROW
        {
            setvar(var_, l, fn);
        }

        explicit
        var(void* p, void (*fn)(void*) = 0) AUG_NOTHROW
        {
            setvar(var_, p, fn);
        }

        var&
        operator =(const null_&) AUG_NOTHROW
        {
            clearvar(var_);
            return *this;
        }

        operator aug_var&()
        {
            return var_;
        }

        operator const aug_var&() const
        {
            return var_;
        }
    };

    inline bool
    operator ==(const aug_var& lhs, const aug_var& rhs)
    {
        return 0 != aug_equalvar(&lhs, &rhs) ? true : false;
    }

    inline bool
    operator !=(const aug_var& lhs, const aug_var& rhs)
    {
        return 0 == aug_equalvar(&lhs, &rhs) ? true : false;
    }
}

inline bool
isnull(const aug_var& v)
{
    return 0 != aug_isnull(&v) ? true : false;
}

#endif // AUGUTILPP_VAR_HPP
