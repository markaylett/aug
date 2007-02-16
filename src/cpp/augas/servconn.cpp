/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/servconn.hpp"

using namespace aug;
using namespace augas;
using namespace std;

void
servconn::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    rwtimer_.callback(ref, ms, timers);
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
servconn::do_connected(const aug_endpoint& ep)
{
    conn_.connected(ep);
}

bool
servconn::do_process(mplexer& mplexer)
{
    return conn_.process(mplexer);
}

void
servconn::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    conn_.putsome(mplexer, buf, size);
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
    sock_.serv_ = cptr(*serv);
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
