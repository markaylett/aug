/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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
        verify(aug_strtoul(&ul, src, base));
        return ul;
    }

    inline unsigned
    strtoui(const char* src, int base)
    {
        unsigned ui;
        verify(aug_strtoui(&ui, src, base));
        return ui;
    }

    inline unsigned short
    strtous(const char* src, int base)
    {
        unsigned short us;
        verify(aug_strtous(&us, src, base));
        return us;
    }
}

#endif // AUGUTILPP_CONV_HPP
