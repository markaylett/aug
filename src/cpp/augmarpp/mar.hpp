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
#ifndef AUGMARPP_MAR_HPP
#define AUGMARPP_MAR_HPP

#include "augctxpp/exception.hpp"

#include "augmar/mar.h"

#include <string>

namespace aug {

    inline marptr
    createmar(mpoolref mpool)
    {
        return object_attach<aug_mar>(verify(aug_createmar(mpool.get())));
    }
    inline marptr
    openmar(mpoolref mpool, const char* path, int flags)
    {
        return object_attach<aug_mar>
            (verify(aug_openmar(mpool.get(), path, flags)));
    }
    inline marptr
    openmar(mpoolref mpool, const char* path, int flags, mode_t mode)
    {
        return object_attach<aug_mar>
            (verify(aug_openmar(mpool.get(), path, flags, mode)));
    }
    inline void
    putfieldn(marref ref, unsigned n, const char* cdata)
    {
        putfieldn(ref, n, cdata, static_cast<unsigned>(strlen(cdata)));
    }
    inline void
    putfieldn(marref ref, unsigned n, const std::string& cdata)
    {
        putfieldn(ref, n, cdata.data(), static_cast<unsigned>(cdata.size()));
    }
    inline void
    putfieldp(marref ref, const char* name, const char* cdata)
    {
        putfieldp(ref, name, cdata, static_cast<unsigned>(strlen(cdata)));
    }
    inline void
    putfieldp(marref ref, const char* name, const std::string& cdata)
    {
        putfieldp(ref, name, cdata.data(),
                  static_cast<unsigned>(cdata.size()));
    }
    inline void
    setcontent(marref ref, const char* data)
    {
        setcontent(ref, data, static_cast<unsigned>(strlen(data)));
    }
    inline void
    setcontent(marref ref, const std::string& data)
    {
        setcontent(ref, data.data(), static_cast<unsigned>(data.size()));
    }
}

#endif // AUGMARPP_MAR_HPP
