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
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augctxpp/exception.hpp"

#include "augutil/file.h"

#include "augctx/errno.h"

#include "augext/log.h"

namespace aug {

    template <aug_result (*T)(const char*, const char*, void*)>
    aug_result
    confcb(const char* name, const char* value, void* arg) AUG_NOTHROW
    {
        try {
            return T(name, value, arg);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T, aug_result (T::*U)(const char*, const char*)>
    aug_result
    confmemcb(const char* name, const char* value, void* arg) AUG_NOTHROW
    {
        try {
            return (static_cast<T*>(arg)->*U)(name, value);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    template <typename T>
    aug_result
    confmemcb(const char* name, const char* value, void* arg) AUG_NOTHROW
    {
        try {
            return static_cast<T*>(arg)->confcb(name, value);
        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    inline aug_result
    readconf(const char* path, aug_confcb_t cb, void* arg)
    {
        return verify(aug_readconf(path, cb, arg));
    }

    inline aug_result
    readconf(const char* path, aug_confcb_t cb, const null_&)
    {
        return verify(aug_readconf(path, cb, 0));
    }

    template <typename T>
    aug_result
    readconf(const char* path, T& x)
    {
        return verify(aug_readconf(path, confmemcb<T>, &x));
    }

    inline aug_result
    readconf(FILE* fp, aug_confcb_t cb, void* arg)
    {
        return verify(aug_freadconf(fp, cb, arg));
    }

    inline aug_result
    readconf(FILE* fp, aug_confcb_t cb, const null_&)
    {
        return verify(aug_freadconf(fp, cb, 0));
    }

    template <typename T>
    aug_result
    readconf(FILE* fp, T& x)
    {
        return verify(aug_freadconf(fp, confmemcb<T>, &x));
    }
}

#endif // AUGUTILPP_FILE_HPP
