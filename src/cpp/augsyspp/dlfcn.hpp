/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYSPP_DLFCN_HPP
#define AUGSYSPP_DLFCN_HPP

#include "augctxpp/exception.hpp"
#include "augctxpp/utility.hpp" // perrinfo()

#include "augctxpp/mpool.hpp"

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

    class dlib : public mpool_ops {

        aug_dlib_t dlib_;

        dlib(const dlib&);

        dlib&
        operator =(const dlib&);

        void
        reset(aug_dlib_t dl) AUG_NOTHROW
        {
            aug_dlib_t prev(dlib_);
            dlib_ = dl;
            if (prev && AUG_ISFAIL(aug_dlclose(prev)))
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
