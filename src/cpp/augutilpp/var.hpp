/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_VAR_HPP
#define AUGUTILPP_VAR_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/errinfo.hpp"
#include "augsyspp/exception.hpp"

#include "augutil/var.h"

namespace aug {

    template <typename T>
    struct basic_vartype {
        typedef T arg_type;
    protected:
        ~basic_vartype() AUG_NOTHROW
        {
        }
    public:
        static void
        destroy(T* arg)
        {
        }
        static const void*
        buf(T& arg, size_t& size)
        {
            return 0;
        }
        static const void*
        buf(T& arg)
        {
            return 0;
        }
    };

    namespace detail {

        template <typename T>
        class vartype {
            static int
            destroy(void* arg) AUG_NOTHROW
            {
                try {
                    T::destroy(static_cast<typename T::arg_type*>(arg));
                    return 0;
                } AUG_SETERRINFOCATCH;
                return -1;
            }
            static const void*
            buf(void* arg, size_t* size) AUG_NOTHROW
            {
                try {
                    return size
                        ? T::buf(*static_cast<typename T::arg_type*>(arg),
                                 *size)
                        : T::buf(*static_cast<typename T::arg_type*>(arg));
                } AUG_SETERRINFOCATCH;
                return 0;
            }
        public:
            static const aug_vartype&
            get()
            {
                static const aug_vartype local = {
                    destroy,
                    buf
                };
                return local;
            }
        };
    }

    template <typename T>
    aug_var&
    bindvar(aug_var& var, typename T::arg_type& arg)
    {
        var.type_ = &detail::vartype<T>::get();
        var.arg_ = &arg;
        return var;
    }

    template <typename T>
    aug_var&
    bindvar(aug_var& var, const null_&)
    {
        var.type_ = &detail::vartype<T>::get();
        var.arg_ = 0;
        return var;
    }

    inline void
    destroyvar(const aug_var& var)
    {
        if (-1 == aug_destroyvar(&var))
            perrinfo("aug_destroyvar() failed");
    }

    inline const void*
    varbuf(const aug_var& var, size_t& size)
    {
        return aug_varbuf(&var, &size);
    }

    template <typename T>
    const T*
    varbuf(const aug_var& var, size_t& size)
    {
        return static_cast<const T*>(varbuf(var, size));
    }

    inline const void*
    varbuf(const aug_var& var)
    {
        return aug_varbuf(&var, 0);
    }

    template <typename T>
    const T*
    varbuf(const aug_var& var)
    {
        return static_cast<const T*>(varbuf(var));
    }
}

inline bool
isnull(const aug_var& var)
{
    return var.arg_ ? false : true;
}

#endif // AUGUTILPP_VAR_HPP
