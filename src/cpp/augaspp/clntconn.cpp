/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
#include "augaspp/clntconn.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

mod_handle&
clntconn::do_get()
{
    return impl_.get();
}

const mod_handle&
clntconn::do_get() const
{
    return impl_.get();
}

const sessionptr&
clntconn::do_session() const
{
    return impl_.session();
}

void
clntconn::do_send(chanref chan, const void* buf, size_t len,
                  const timeval& now)
{
    impl_.send(chan, buf, len, now);
}

void
clntconn::do_sendv(chanref chan, blobref blob, const timeval& now)
{
    impl_.sendv(chan, blob, now);
}

bool
clntconn::do_accepted(const string& name, const timeval& now)
{
    return impl_.accepted(name, now);
}

void
clntconn::do_connected(const string& name, const timeval& now)
{
    impl_.connected(name, now);
}

void
clntconn::do_process(chanref chan, unsigned short events, const timeval& now)
{
    impl_.process(chan, events, now);
}

void
clntconn::do_shutdown(chanref chan, unsigned flags, const timeval& now)
{
    impl_.shutdown(chan, flags, now);
}

void
clntconn::do_teardown(const timeval& now)
{
    impl_.teardown(now);
}

bool
clntconn::do_authcert(const char* subject, const char* issuer)
{
    return impl_.authcert(subject, issuer);
}

string
clntconn::do_peername(chanref chan) const
{
    return impl_.peername(chan);
}

sockstate
clntconn::do_state() const
{
    return impl_.state();
}

void
clntconn::do_timercb(idref id, unsigned& ms)
{
    rwtimer_.timercb(id, ms);
}

void
clntconn::do_setrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.setrwtimer(ms, flags);
}

bool
clntconn::do_resetrwtimer(unsigned ms, unsigned flags)
{
    return rwtimer_.resetrwtimer(ms, flags);
}

bool
clntconn::do_resetrwtimer(unsigned flags)
{
    return rwtimer_.resetrwtimer(flags);
}

bool
clntconn::do_cancelrwtimer(unsigned flags)
{
    return rwtimer_.cancelrwtimer(flags);
}

clntconn::~clntconn() AUG_NOTHROW
{
}

clntconn::clntconn(mpoolref mpool, const sessionptr& session, void* user,
                   timers& timers, unsigned id)
    : impl_(session, sock_, buffer_, rwtimer_, true), // See comment.
      buffer_(mpool),
      rwtimer_(session, sock_, timers)
{
    // Client connections are implicitly accepted as the client is initiating.

    sock_.id_ = id;
    sock_.user_ = user;
}
