/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/rwtimer.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <cassert>

using namespace aug;
using namespace std;

rwtimer_base::~rwtimer_base() AUG_NOTHROW
{
}

void
rwtimer::do_timercb(int id, unsigned& ms)
{
    if (rdtimer_.id() == id) {
        AUG_DEBUG2("read timer expiry");
        session_->rdexpire(sock_, ms);
    } else if (wrtimer_.id() == id) {
        AUG_DEBUG2("write timer expiry");
        session_->wrexpire(sock_, ms);
    } else
        assert(0);
}

void
rwtimer::do_setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGMOD_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & AUGMOD_TIMWR)
        wrtimer_.set(ms, *this);
}

bool
rwtimer::do_resetrwtimer(unsigned ms, unsigned flags)
{
    bool exists(true);

    if (flags & AUGMOD_TIMRD)
        if (!rdtimer_.reset(ms))
            exists = false;

    if (flags & AUGMOD_TIMWR)
        if (!wrtimer_.reset(ms))
            exists = false;

    return exists;
}

bool
rwtimer::do_resetrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & AUGMOD_TIMRD)
        if (!rdtimer_.reset())
            exists = false;

    if (flags & AUGMOD_TIMWR)
        if (!wrtimer_.reset())
            exists = false;

    return exists;
}

bool
rwtimer::do_cancelrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & AUGMOD_TIMRD)
        if (!rdtimer_.cancel())
            exists = false;

    if (flags & AUGMOD_TIMWR)
        if (!wrtimer_.cancel())
            exists = false;

    return exists;
}

rwtimer::~rwtimer() AUG_NOTHROW
{
}

rwtimer::rwtimer(const sessionptr& session, const maud_object& sock,
                 timers& timers)
    : session_(session),
      sock_(sock),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}
