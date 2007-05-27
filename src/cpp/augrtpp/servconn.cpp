/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/servconn.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

AUGRTPP_API void
servconn::do_timercb(int id, unsigned& ms)
{
    rwtimer_.timercb(id, ms);
}

AUGRTPP_API void
servconn::do_setrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.setrwtimer(ms, flags);
}

AUGRTPP_API bool
servconn::do_resetrwtimer(unsigned ms, unsigned flags)
{
    return rwtimer_.resetrwtimer(ms, flags);
}

AUGRTPP_API bool
servconn::do_resetrwtimer(unsigned flags)
{
    return rwtimer_.resetrwtimer(flags);
}

AUGRTPP_API bool
servconn::do_cancelrwtimer(unsigned flags)
{
    return rwtimer_.cancelrwtimer(flags);
}

AUGRTPP_API augas_object&
servconn::do_get()
{
    return conn_.get();
}

AUGRTPP_API const augas_object&
servconn::do_get() const
{
    return conn_.get();
}

AUGRTPP_API const servptr&
servconn::do_serv() const
{
    return conn_.serv();
}

AUGRTPP_API smartfd
servconn::do_sfd() const
{
    return conn_.sfd();
}

AUGRTPP_API void
servconn::do_send(const void* buf, size_t len)
{
    conn_.send(buf, len);
}

AUGRTPP_API void
servconn::do_sendv(const aug_var& var)
{
    conn_.sendv(var);
}

AUGRTPP_API bool
servconn::do_accepted(const aug_endpoint& ep)
{
    return conn_.accepted(ep);
}

AUGRTPP_API void
servconn::do_connected(const aug_endpoint& ep)
{
    // BUG: workaround for bug in gcc version 3.4.4.

    conn_base& r(conn_);
    r.connected(ep);
}

AUGRTPP_API bool
servconn::do_process(unsigned short events)
{
    return conn_.process(events);
}

AUGRTPP_API void
servconn::do_shutdown()
{
    conn_.shutdown();
}

AUGRTPP_API void
servconn::do_teardown()
{
    conn_.teardown();
}

AUGRTPP_API bool
servconn::do_authcert(const char* subject, const char* issuer)
{
    return conn_.authcert(subject, issuer);
}

AUGRTPP_API const endpoint&
servconn::do_peername() const
{
    return conn_.peername();
}

AUGRTPP_API sockstate
servconn::do_state() const
{
    return conn_.state();
}

AUGRTPP_API
servconn::~servconn() AUG_NOTHROW
{
}

AUGRTPP_API
servconn::servconn(const servptr& serv, void* user, timers& timers,
                   const smartfd& sfd, const endpoint& ep)
    : rwtimer_(serv, sock_, timers),
      conn_(serv, sock_, buffer_, rwtimer_, sfd, ep, false)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
