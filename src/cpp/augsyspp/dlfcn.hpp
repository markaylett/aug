/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_DLFCN_HPP
#define AUGSYSPP_DLFCN_HPP

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

#include "augnullpp.hpp"

#include "augsys/dlfcn.h"

namespace aug {

    inline aug_fnptr_t
    dlsym(aug_dlib_t dl, const char* symbol)
    {
        return verify(aug_dlsym(dl, symbol));
    }

    template <typename fnT>
    fnT
    dlsym(aug_dlib_t dl, const char* symbol)
    {
        return (fnT)dlsym(dl, symbol);
    }

    class dlib {

        aug_dlib_t dlib_;

        dlib(const dlib&);

        dlib&
        operator =(const dlib&);

        void
        reset(aug_dlib_t dl) AUG_NOTHROW
        {
            aug_dlib_t prev(dlib_);
            dlib_ = dl;
            if (prev && -1 == aug_dlclose(prev))
                perrinfo(aug_tlx, "aug_dlclose() failed");
        }

    public:
        ~dlib() AUG_NOTHROW
        {
            reset(0);
        }

        dlib(const null_&) AUG_NOTHROW
            : dlib_(0)
        {
        }

        dlib(mpoolref mpool, const char* path)
            : dlib_(0)
        {
            open(mpool, path);
        }

        void
        close()
        {
            if (dlib_) {
                aug_dlib_t prev(dlib_);
                dlib_ = 0;
                verify(aug_dlclose(prev));
            }
        }

        void
        open(mpoolref mpool, const char* path)
        {
            aug_dlib_t dl(aug_dlopen(mpool.get(), path));
            verify(dl);
            reset(dl);
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
isnull(aug_dlib_t dl)
{
    return 0 == dl;
}

#endif // AUGSYSPP_DLFCN_HPP
