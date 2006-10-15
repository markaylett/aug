/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_DLFCN_HPP
#define AUGSYSPP_DLFCN_HPP

#include "augsyspp/exception.hpp"

#include "augsys/dlfcn.h"

namespace aug {

    inline void*
    dlsym(aug_dlib_t dlib, const char* symbol)
    {
        void* fn(aug_dlsym(dlib, symbol));
        if (!fn)
            throwerrinfo("aug_dlsym() failed");
        return fn;
    }

    template <typename fnT>
    fnT
    dlsym(aug_dlib_t dlib, const char* symbol)
    {
        return (fnT)(dlsym(dlib, symbol));
    }

    class dlib {

        aug_dlib_t dlib_;

        dlib(const dlib&);

        dlib&
        operator =(const dlib&);

    public:
        ~dlib() AUG_NOTHROW
        {
            if (-1 == aug_dlclose(dlib_))
                aug_perrinfo("aug_dlclose() failed");
        }

        explicit
        dlib(const char* path)
            : dlib_(aug_dlopen(path))
        {
            if (!dlib_)
                throwerrinfo("aug_dlopen() failed");
        }

        operator aug_dlib_t()
        {
            return dlib_;
        }

        aug_dlib_t
        get()
        {
            return dlib_;
        }
    };
}

#endif // AUGSYSPP_DLFCN_HPP
