/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/servconn.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

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

mod_handle&
servconn::do_get()
{
    return conn_.get();
}

const mod_handle&
servconn::do_get() const
{
    return conn_.get();
}

const sessionptr&
servconn::do_session() const
{
    return conn_.session();
}

chanptr
servconn::do_chan() const
{
    return conn_.sd();
}

void
servconn::do_send(const void* buf, size_t len, const timeval& now)
{
    conn_.send(buf, len, now);
}

void
servconn::do_sendv(blobref blob, const timeval& now)
{
    conn_.sendv(blob, now);
}

bool
servconn::do_accepted(const string& name, const timeval& now)
{
    return conn_.accepted(name, now);
}

void
servconn::do_connected(const string& name, const timeval& now)
{
    // BUG: workaround for bug in gcc version 3.4.4.

    conn_base& r(conn_);
    r.connected(name, now);
}

bool
servconn::do_process(unsigned short events, const timeval& now)
{
    return conn_.process(events, now);
}

void
servconn::do_shutdown(unsigned flags, const timeval& now)
{
    conn_.shutdown(flags, now);
}

void
servconn::do_teardown(const timeval& now)
{
    conn_.teardown(now);
}

bool
servconn::do_authcert(const char* subject, const char* issuer)
{
    return conn_.authcert(subject, issuer);
}

string
servconn::do_peername() const
{
    return conn_.peername();
}

sockstate
servconn::do_state() const
{
    return conn_.state();
}

servconn::~servconn() AUG_NOTHROW
{
}

servconn::servconn(const sessionptr& session, void* user, timers& timers,
                   autosd& sd, const endpoint& ep)
    : rwtimer_(session, sock_, timers),
      conn_(session, sock_, buffer_, rwtimer_, sd, ep, false)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
