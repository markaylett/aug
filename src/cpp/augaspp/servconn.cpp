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
#include "augaspp/servconn.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

mod_handle&
servconn::do_get()
{
    return impl_.get();
}

const mod_handle&
servconn::do_get() const
{
    return impl_.get();
}

const sessionptr&
servconn::do_session() const
{
    return session_;
}

void
servconn::do_error(const char* desc)
{
    impl_.error(desc);
}

void
servconn::do_shutdown(chanref chan, unsigned flags, const aug_timeval& now)
{
    impl_.shutdown(chan, flags, now);
}

sockstate
servconn::do_state() const
{
    return impl_.state();
}

void
servconn::do_send(chanref chan, const void* buf, size_t len,
                  const aug_timeval& now)
{
    impl_.send(chan, buf, len, now);
}

void
servconn::do_sendv(chanref chan, blobref blob, const aug_timeval& now)
{
    impl_.sendv(chan, blob, now);
}

mod_bool
servconn::do_accepted(const string& name, const aug_timeval& now)
{
    return impl_.accepted(name, now);
}

void
servconn::do_connected(const string& name, const aug_timeval& now)
{
    impl_.connected(name, now);
}

mod_bool
servconn::do_auth(const char* subject, const char* issuer)
{
    return impl_.auth(subject, issuer);
}

void
servconn::do_process_BI(chanref chan, unsigned short events,
                        const aug_timeval& now)
{
    impl_.process_BI(chan, events, now);
}

void
servconn::do_teardown(const aug_timeval& now)
{
    impl_.teardown(now);
}

string
servconn::do_peername(chanref chan) const
{
    return impl_.peername(chan);
}

void
servconn::do_timercb(idref id, unsigned& ms)
{
    rwtimer_.timercb(id, ms);
}

void
servconn::do_setrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.setrwtimer(ms, flags);
}

bool
servconn::do_resetrwtimer(unsigned ms, unsigned flags)
{
    return rwtimer_.resetrwtimer(ms, flags);
}

bool
servconn::do_resetrwtimer(unsigned flags)
{
    return rwtimer_.resetrwtimer(flags);
}

bool
servconn::do_cancelrwtimer(unsigned flags)
{
    return rwtimer_.cancelrwtimer(flags);
}

servconn::~servconn() AUG_NOTHROW
{
}

servconn::servconn(mpoolref mpool, const sessionptr& session,
                   aug_timers_t timers, aug_id id, objectref ob)
    : session_(session),
      sock_(id, ob),
      buffer_(mpool),
      rwtimer_(*session, sock_, timers),
      impl_(*session, sock_, buffer_, rwtimer_, false) // See comment.
{
    // New server connection: needs session acceptance.
}
