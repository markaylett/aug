/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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

    inline char*
    makepath(char* dst, const char* dir, const char* name, const char* ext,
             size_t size)
    {
        return verify(aug_makepath(dst, dir, name, ext, size));
    }

    inline std::string
    makepath(const char* dir, const char* name, const char* ext)
    {
        char buf[AUG_PATH_MAX + 1];
        return makepath(buf, dir, name, ext, sizeof(buf));
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
