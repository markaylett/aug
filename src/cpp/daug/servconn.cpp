/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "daug/servconn.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace augas;
using namespace std;

void
servconn::do_timercb(int id, unsigned& ms)
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

augas_object&
servconn::do_object()
{
    return conn_.object();
}

const augas_object&
servconn::do_object() const
{
    return conn_.object();
}

const servptr&
servconn::do_serv() const
{
    return conn_.serv();
}

smartfd
servconn::do_sfd() const
{
    return conn_.sfd();
}

bool
servconn::do_accept(const aug_endpoint& ep)
{
    return conn_.accept(ep);
}

void
servconn::do_append(const aug_var& var)
{
    conn_.append(var);
}

void
servconn::do_append(const void* buf, size_t len)
{
    conn_.append(buf, len);
}

void
servconn::do_connected(const aug_endpoint& ep)
{
    conn_.connected(ep);
}

bool
servconn::do_process(unsigned short events)
{
    return conn_.process(events);
}

void
servconn::do_shutdown()
{
    conn_.shutdown();
}

void
servconn::do_teardown()
{
    conn_.teardown();
}

bool
servconn::do_authcert(const char* subject, const char* issuer)
{
    return conn_.authcert(subject, issuer);
}

const endpoint&
servconn::do_endpoint() const
{
    return conn_.endpoint();
}

connphase
servconn::do_phase() const
{
    return conn_.phase();
}

servconn::~servconn() AUG_NOTHROW
{
}

servconn::servconn(const servptr& serv, void* user, timers& timers,
                   const smartfd& sfd, const aug::endpoint& ep)
    : rwtimer_(serv, sock_, timers),
      conn_(serv, sock_, buffer_, rwtimer_, sfd, ep, false)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
