/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/rwtimer.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <cassert>

using namespace aug;
using namespace std;

AUGRTPP_API
rwtimer_base::~rwtimer_base() AUG_NOTHROW
{
}

AUGRTPP_API void
rwtimer::do_timercb(int id, unsigned& ms)
{
    if (rdtimer_.id() == id) {
        AUG_DEBUG2("read timer expiry");
        serv_->rdexpire(sock_, ms);
    } else if (wrtimer_.id() == id) {
        AUG_DEBUG2("write timer expiry");
        serv_->wrexpire(sock_, ms);
    } else
        assert(0);
}

AUGRTPP_API void
rwtimer::do_setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & AUGAS_TIMWR)
        wrtimer_.set(ms, *this);
}

AUGRTPP_API bool
rwtimer::do_resetrwtimer(unsigned ms, unsigned flags)
{
    bool exists(true);

    if (flags & AUGAS_TIMRD)
        if (!rdtimer_.reset(ms))
            exists = false;

    if (flags & AUGAS_TIMWR)
        if (!wrtimer_.reset(ms))
            exists = false;

    return exists;
}

AUGRTPP_API bool
rwtimer::do_resetrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & AUGAS_TIMRD)
        if (!rdtimer_.reset())
            exists = false;

    if (flags & AUGAS_TIMWR)
        if (!wrtimer_.reset())
            exists = false;

    return exists;
}

AUGRTPP_API bool
rwtimer::do_cancelrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & AUGAS_TIMRD)
        if (!rdtimer_.cancel())
            exists = false;

    if (flags & AUGAS_TIMWR)
        if (!wrtimer_.cancel())
            exists = false;

    return exists;
}

AUGRTPP_API
rwtimer::~rwtimer() AUG_NOTHROW
{
}

AUGRTPP_API
rwtimer::rwtimer(const servptr& serv, const augas_object& sock,
                 timers& timers)
    : serv_(serv),
      sock_(sock),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}
