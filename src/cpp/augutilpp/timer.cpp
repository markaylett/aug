/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTILPP_BUILD
#include "augutilpp/timer.hpp"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/string.h" // aug_perror()

using namespace aug;

AUGUTILPP_API
timers::~timers() NOTHROW
{
    if (-1 == aug_freetimers(&timers_))
        aug_perror("aug_freetimers() failed");
}

AUGUTILPP_API
timers::timers()
{
    AUG_INIT(&timers_);
}

AUGUTILPP_API bool
timers::empty() const
{
    return AUG_EMPTY(&timers_);
}

AUGUTILPP_API
expire_base::~expire_base() NOTHROW
{
}

AUGUTILPP_API void
timer::expire_(void* arg, int id)
{
    try {
        timer* ptr = static_cast<timer*>(arg);
        ptr->pending_ = false;
        ptr->action_.expire(id);
    } AUG_CATCHRETURN;
}

AUGUTILPP_API
timer::~timer() NOTHROW
{
    if (pending() && -1 == aug_canceltimer(&timers_, id_))
        aug_perror("aug_canceltimer() failed");
}

AUGUTILPP_API void
timer::cancel()
{
    if (pending()) {

        int id(id_);
        id_ = -1;
        pending_ = false;

        if (-1 == aug_canceltimer(&timers_, id))
            error("aug_canceltimer() failed");
    }
}

AUGUTILPP_API void
timer::reset(unsigned int ms)
{
    cancel();
    int id(aug_settimer(&timers_, ms, expire_, this));
    if (-1 == id)
        error("aug_settimer() failed");
    id_ = id;
    pending_ = true;
}
