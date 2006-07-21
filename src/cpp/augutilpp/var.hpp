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
        struct var_traits {
            static struct aug_var&
            set(struct aug_var& v, T l)
            {
                return *aug_setvarl(&v, static_cast<long>(l));
            }
            static T
            get(const struct aug_var& v)
            {
                return static_cast<T>(aug_getvarl(&v));
            }
        };

        template <typename T>
        struct var_traits<T*> {
            static struct aug_var&
            set(struct aug_var& v, T* p)
            {
                return *aug_setvarp(&v, p);
            }
            static T*
            get(const struct aug_var& v)
            {
                return static_cast<T*>(aug_getvarp(&v));
            }
        };

        template <>
        struct var_traits<aug_var> {
            static struct aug_var&
            set(struct aug_var& v, const struct aug_var& w)
            {
                return *aug_setvar(&v, &w);
            }
        };
    }

    inline aug_var&
    clearvar(struct aug_var& v)
    {
        return *aug_clearvar(&v);
    }

    template <typename T>
    aug_var&
    setvar(struct aug_var& v, T x)
    {
        return detail::var_traits<T>::set(v, x);
    }

    template <typename T>
    T
    getvar(const struct aug_var& v)
    {
        return detail::var_traits<T>::get(v);
    }

    class var {
    public:
        typedef struct aug_var ctype;
    private:
        struct aug_var var_;

        var(const var&);

        var&
        operator =(const var&);

    public:
        var()
        {
            clearvar(var_);
        }

        var(const null_&)
        {
            clearvar(var_);
        }

        template <typename T>
        explicit
        var(T x)
        {
            setvar<T>(var_, x);
        }

        var&
        operator =(const null_&)
        {
            clearvar(var_);
            return *this;
        }

        template <typename T>
        var&
        operator =(T x)
        {
            setvar<T>(var_, x);
            return *this;
        }

        operator struct aug_var&()
        {
            return var_;
        }

        operator const struct aug_var&() const
        {
            return var_;
        }

        aug_vartype
        type() const
        {
            return var_.type_;
        }
    };

    inline bool
    operator ==(const var& lhs, const var& rhs)
    {
        return 0 != aug_equalvar(cptr(lhs), cptr(rhs)) ? true : false;
    }

    inline bool
    operator !=(const var& lhs, const var& rhs)
    {
        return 0 == aug_equalvar(cptr(lhs), cptr(rhs)) ? true : false;
    }
}

inline bool
isnull(const aug::var& v)
{
    return 0 != aug_isnull(cptr(v)) ? true : false;
}

#endif // AUGUTILPP_VAR_HPP
