/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_VAR_HPP
#define AUGUTILPP_VAR_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/errinfo.hpp"

#include "augutil/var.h"

namespace aug {

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
        return static_cast<T*>(varbuf(var, size));
    }
}

inline bool
isnull(const aug_var& var)
{
    return var.ptr_ ? false : true;
}

#endif // AUGUTILPP_VAR_HPP
