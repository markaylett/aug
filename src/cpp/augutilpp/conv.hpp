/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CONV_HPP
#define AUGUTILPP_CONV_HPP

#include "augutilpp/config.hpp"

#include "augutil/conv.h"

#include <string>

namespace aug {

    AUGUTILPP_API unsigned long
    strtoul(const char* src, int base);

    AUGUTILPP_API unsigned int
    strtoui(const char* src, int base);

    AUGUTILPP_API unsigned short
    strtous(const char* src, int base);
}

#endif // AUGUTILPP_CONV_HPP
