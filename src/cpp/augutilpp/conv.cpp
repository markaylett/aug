/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTILPP_BUILD
#include "augutilpp/conv.hpp"

#include "augsyspp/exception.hpp"

using namespace aug;
using namespace std;

AUGUTILPP_API unsigned long
aug::strtoul(const char* src, int base)
{
    unsigned long ul;
    if (-1 == aug_strtoul(&ul, src, base))
        error("aug_strtoul() failed");
    return ul;
}

AUGUTILPP_API unsigned int
aug::strtoui(const char* src, int base)
{
    unsigned int ui;
    if (-1 == aug_strtoui(&ui, src, base))
        error("aug_strtoui() failed");
    return ui;
}

AUGUTILPP_API unsigned short
aug::strtous(const char* src, int base)
{
    unsigned short us;
    if (-1 == aug_strtous(&us, src, base))
        error("aug_strtous() failed");
    return us;
}
