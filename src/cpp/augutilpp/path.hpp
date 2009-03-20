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
#ifndef AUGUTILPP_PATH_HPP
#define AUGUTILPP_PATH_HPP

#include "augutilpp/config.hpp"

#include "augctxpp/exception.hpp"

#include "augutil/path.h"

#include "augsys/limits.h"

#include <string>

namespace aug {

    inline void
    chdir(const char* path)
    {
        verify(aug_chdir(path));
    }

    inline char*
    getcwd(char* dst, size_t size)
    {
        return verify(aug_getcwd(dst, size));
    }

    inline std::string
    getcwd()
    {
        char buf[AUG_PATH_MAX + 1];
        return getcwd(buf, sizeof(buf));
    }

    inline bool
    isabs(const char* path)
    {
        return aug_isabs(path) ? true : false;
    }

    inline char*
    abspath(char* dst, const char* dir, const char* path, size_t size)
    {
        return verify(aug_abspath(dst, dir, path, size));
    }

    inline std::string
    abspath(const char* dir, const char* path)
    {
        char buf[AUG_PATH_MAX + 1];
        return abspath(buf, dir, path, sizeof(buf));
    }

    inline char*
    joinpath(char* dst, const char* dir, const char* path, size_t size)
    {
        return verify(aug_joinpath(dst, dir, path, size));
    }

    inline std::string
    joinpath(const char* dir, const char* path)
    {
        char buf[AUG_PATH_MAX + 1];
        return joinpath(buf, dir, path, sizeof(buf));
    }

    inline char*
    realpath(char* dst, const char* src, size_t size)
    {
        return verify(aug_realpath(dst, src, size));
    }

    inline std::string
    realpath(const char* path)
    {
        char buf[AUG_PATH_MAX + 1];
        return realpath(buf, path, sizeof(buf));
    }
}

#endif // AUGUTILPP_PATH_HPP
