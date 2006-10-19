/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_PATH_HPP
#define AUGUTILPP_PATH_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/path.h"

#include "augsys/limits.h"

#include <string>

namespace aug {

    inline void
    chdir(const char* path)
    {
        if (-1 == aug_chdir(path))
            throwerrinfo("aug_chdir() failed");
    }

    inline char*
    getcwd(char* dst, size_t size)
    {
        if (!aug_getcwd(dst, size))
            throwerrinfo("aug_getcwd() failed");
        return dst;
    }

    inline std::string
    getcwd()
    {
        char buf[AUG_PATH_MAX + 1];
        return getcwd(buf, sizeof(buf));
    }

    inline char*
    realpath(char* dst, const char* src, size_t size)
    {
        if (!aug_realpath(dst, src, size))
            throwerrinfo("aug_realpath() failed");
        return dst;
    }

    inline std::string
    realpath(const char* path)
    {
        char buf[AUG_PATH_MAX + 1];
        return realpath(buf, path, sizeof(buf));
    }
}

#endif // AUGUTILPP_PATH_HPP
