/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CONV_HPP
#define AUGUTILPP_CONV_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/conv.h"

#include <string>

namespace aug {

    inline unsigned long
    strtoul(const char* src, int base)
    {
        unsigned long ul;
        if (-1 == aug_strtoul(&ul, src, base))
            error("aug_strtoul() failed");
        return ul;
    }

    inline unsigned int
    strtoui(const char* src, int base)
    {
        unsigned int ui;
        if (-1 == aug_strtoui(&ui, src, base))
            error("aug_strtoui() failed");
        return ui;
    }

    inline unsigned short
    strtous(const char* src, int base)
    {
        unsigned short us;
        if (-1 == aug_strtous(&us, src, base))
            error("aug_strtous() failed");
        return us;
    }
}

#endif // AUGUTILPP_CONV_HPP
