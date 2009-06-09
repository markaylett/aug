/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGASPP_BUILD
#include "augaspp/rwtimer.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <cassert>

using namespace aug;
using namespace std;

rwtimer_base::~rwtimer_base() AUG_NOTHROW
{
}

void
rwtimer::do_timercb(idref id, unsigned& ms)
{
    if (rdtimer_.id() == id) {
        AUG_CTXDEBUG2(aug_tlx, "read timer expiry");
        session_->rdexpire(sock_, ms);
    } else if (wrtimer_.id() == id) {
        AUG_CTXDEBUG2(aug_tlx, "write timer expiry");
        session_->wrexpire(sock_, ms);
    } else
        assert(0);
}

void
rwtimer::do_setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & MOD_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & MOD_TIMWR)
        wrtimer_.set(ms, *this);
}

bool
rwtimer::do_resetrwtimer(unsigned ms, unsigned flags)
{
    bool exists(true);

    if (flags & MOD_TIMRD)
        try {
            rdtimer_.reset(ms);
        } catch (const none_exception&) {
            exists = false;
        }

    if (flags & MOD_TIMWR)
        try {
            wrtimer_.reset(ms);
        } catch (const none_exception&) {
            exists = false;
        }

    return exists;
}

bool
rwtimer::do_resetrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & MOD_TIMRD)
        try {
            rdtimer_.reset();
        } catch (const none_exception&) {
            exists = false;
        }

    if (flags & MOD_TIMWR)
        try {
            wrtimer_.reset();
        } catch (const none_exception&) {
            exists = false;
        }

    return exists;
}

bool
rwtimer::do_cancelrwtimer(unsigned flags)
{
    bool exists(true);

    if (flags & MOD_TIMRD)
        try {
            rdtimer_.cancel();
        } catch (const none_exception&) {
            exists = false;
        }

    if (flags & MOD_TIMWR)
        try {
            wrtimer_.cancel();
        } catch (const none_exception&) {
            exists = false;
        }

    return exists;
}

rwtimer::~rwtimer() AUG_NOTHROW
{
}

rwtimer::rwtimer(const sessionptr& session, mod_handle& sock,
                 aug_timers_t timers)
    : session_(session),
      sock_(sock),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}
