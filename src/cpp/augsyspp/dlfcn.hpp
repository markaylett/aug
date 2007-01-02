/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_DLFCN_HPP
#define AUGSYSPP_DLFCN_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/null.hpp"

#include "augsys/dlfcn.h"

namespace aug {

    inline aug_fnptr_t
    dlsym(aug_dlib_t dlib, const char* symbol)
    {
        return verify(aug_dlsym(dlib, symbol));
    }

    template <typename fnT>
    fnT
    dlsym(aug_dlib_t dlib, const char* symbol)
    {
        return (fnT)dlsym(dlib, symbol);
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
                perrinfo("aug_dlclose() failed");
        }

        dlib(const null_&) AUG_NOTHROW
        : dlib_(0)
        {
        }

        explicit
        dlib(const char* path)
        {
            open(path);
        }

        void
        close()
        {
            if (dlib_) {
                aug_dlib_t dlib(dlib_);
                dlib_ = 0;
                verify(aug_dlclose(dlib));
            }
        }

        void
        open(const char* path)
        {
            verify(dlib_ = aug_dlopen(path));
        }

        dlib&
        operator =(const null_&)
        {
            close();
            return *this;
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

inline bool
isnull(aug_dlib_t dlib)
{
    return 0 == dlib;
}

#endif // AUGSYSPP_DLFCN_HPP
