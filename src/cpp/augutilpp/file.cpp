/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTILPP_BUILD
#include "augutilpp/file.hpp"

#include "augsyspp/exception.hpp"

#include "augsys/errno.h"
#include "augsys/log.h"

using namespace aug;

namespace {

    int
    setopt_(void* arg, const char* name, const char* value)
    {
        try {
            setopt_base* ptr = static_cast<setopt_base*>(arg);
            ptr->setopt(name, value);
            return 0;
        } AUG_CATCHRETURN -1;
    }
}

AUGUTILPP_API
setopt_base::~setopt_base() NOTHROW
{
}

AUGUTILPP_API void
aug::readconf(const char* path, setopt_base& action)
{
    if (-1 == aug_readconf(path, setopt_, &action))
        error("aug_readconf() failed");
}
