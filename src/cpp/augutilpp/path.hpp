/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_PATH_HPP
#define AUGUTILPP_PATH_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/path.h"

#include <string>

namespace aug {

    inline char*
    realpath(char* dst, const char* src, size_t size)
    {
        if (!aug_realpath(dst, src, size))
            error("aug_realpath() failed");
        return dst;
    }

    AUGUTILPP_API std::string
    realpath(const char* path);
}

#endif // AUGUTILPP_PATH_HPP
