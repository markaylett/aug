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
        sess_->rdexpire(sock_, ms);
    } else if (wrtimer_.id() == ref) {
        AUG_DEBUG2("write timer expiry");
        sess_->wrexpire(sock_, ms);
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

void
rwtimer::do_resetrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.reset(ms);

    if (flags & AUGAS_TIMWR)
        wrtimer_.reset(ms);
}

void
rwtimer::do_resetrwtimer(unsigned flags)
{
    if (flags & AUGAS_TIMRD && null != rdtimer_)
        if (!rdtimer_.reset()) // If timer nolonger exists.
            rdtimer_ = null;

    if (flags & AUGAS_TIMWR && null != wrtimer_)
        if (!wrtimer_.reset()) // If timer nolonger exists.
            wrtimer_ = null;
}

void
rwtimer::do_cancelrwtimer(unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.cancel();

    if (flags & AUGAS_TIMWR)
        wrtimer_.cancel();
}

rwtimer::~rwtimer() AUG_NOTHROW
{
}

rwtimer::rwtimer(const sessptr& sess, const augas_sock& sock, timers& timers)
    : sess_(sess),
      sock_(sock),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}
