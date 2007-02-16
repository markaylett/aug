/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/rwtimer.hpp"

#include <cassert>

using namespace aug;
using namespace augas;
using namespace std;

rwtimer_base::~rwtimer_base() AUG_NOTHROW
{
}

void
rwtimer::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    if (rdtimer_.id() == ref) {
        AUG_DEBUG2("read timer expiry");
        serv_->rdexpire(sock_, ms);
    } else if (wrtimer_.id() == ref) {
        AUG_DEBUG2("write timer expiry");
        serv_->wrexpire(sock_, ms);
    } else
        assert(0);
}

void
rwtimer::do_setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & AUGAS_TIMWR)
        wrtimer_.set(ms, *this);
}

bool
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

bool
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

bool
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

rwtimer::~rwtimer() AUG_NOTHROW
{
}

rwtimer::rwtimer(const servptr& serv, const augas_object& sock,
                 timers& timers)
    : serv_(serv),
      sock_(sock),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}
