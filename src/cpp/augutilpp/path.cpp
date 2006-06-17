/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTILPP_BUILD
#include "augutilpp/path.hpp"

#include "augsys/limits.h"

using namespace aug;
using namespace std;

AUGUTILPP_API string
aug::realpath(const char* path)
{
    char buf[AUG_PATH_MAX + 1];
    if (!aug_realpath(buf, path, sizeof(buf)))
        error("aug_realpath() failed");

    return buf;
}
