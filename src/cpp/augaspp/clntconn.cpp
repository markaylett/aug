/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
#include "augaspp/clntconn.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

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

mod_handle&
clntconn::do_get()
{
    return conn_.get();
}

const mod_handle&
clntconn::do_get() const
{
    return conn_.get();
}

const sessionptr&
clntconn::do_session() const
{
    return conn_.session();
}

chanptr
clntconn::do_chan() const
{
    return conn_.chan();
}

void
clntconn::do_send(const void* buf, size_t len, const timeval& now)
{
    conn_.send(buf, len, now);
}

void
clntconn::do_sendv(blobref ref, const timeval& now)
{
    conn_.sendv(ref, now);
}

bool
clntconn::do_accepted(const string& name, const timeval& now)
{
    return conn_.accepted(name, now);
}

void
clntconn::do_connected(const string& name, const timeval& now)
{
    // BUG: workaround for bug in gcc version 3.4.4.

    conn_base& r(conn_);
    r.connected(name, now);
}

bool
clntconn::do_process(obref<aug_stream> stream, unsigned short events,
                     const timeval& now)
{
    return conn_.process(stream, events, now);
}

void
clntconn::do_shutdown(unsigned flags, const timeval& now)
{
    conn_.shutdown(flags, now);
}

void
clntconn::do_teardown(const timeval& now)
{
    conn_.teardown(now);
}

bool
clntconn::do_authcert(const char* subject, const char* issuer)
{
    return conn_.authcert(subject, issuer);
}

string
clntconn::do_peername() const
{
    return conn_.peername();
}

sockstate
clntconn::do_state() const
{
    return conn_.state();
}

clntconn::~clntconn() AUG_NOTHROW
{
}

clntconn::clntconn(mpoolref mpool, const sessionptr& session, void* user,
                   timers& timers, const chanptr& chan)
    : buffer_(mpool),
      rwtimer_(session, sock_, timers),
      conn_(session, sock_, buffer_, rwtimer_, chan, true)
{
    sock_.id_ = getchanid(chan);
    sock_.user_ = user;
}
