/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_VAR_HPP
#define AUGUTILPP_VAR_HPP

#include "augutilpp/config.hpp"

#include "augutil/var.h"

namespace aug {

    namespace detail {

        template <typename T>
        struct var_traits {
            static struct aug_var&
            setvar(struct aug_var& v, T l)
            {
                return *aug_setvarl(&v, static_cast<long>(l));
            }
            static T
            var(const struct aug_var& v)
            {
                return static_cast<T>(aug_varl(&v));
            }
        };

        template <typename T>
        struct var_traits<T*> {
            static struct aug_var&
            setvar(struct aug_var& v, T* p)
            {
                return *aug_setvarp(&v, p);
            }
            static T*
            var(const struct aug_var& v)
            {
                return static_cast<T*>(aug_varp(&v));
            }
        };

        template <>
        struct var_traits<struct aug_var> {
            static struct aug_var&
            setvar(struct aug_var& v, const struct aug_var& w)
            {
                return *aug_setvar(&v, &w);
            }
        };

        template <typename T>
        aug_var&
        setvar(struct aug_var& v, T x)
        {
            return detail::var_traits<T>::setvar(v, x);
        }

        template <typename T>
        T
        var(const struct aug_var& v)
        {
            return detail::var_traits<T>::var(v);
        }
    }

    class AUGUTILPP_API var {

        struct aug_var var_;

        var(const var&);

        var&
        operator =(const var&);

    public:
        operator struct aug_var&()
        {
            return var_;
        }

        struct aug_var&
        get()
        {
            return var_;
        }
    };
}

#endif // AUGUTILPP_VAR_HPP
