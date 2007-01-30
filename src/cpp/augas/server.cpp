/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/server.hpp"

using namespace aug;
using namespace augas;
using namespace std;

void
server::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    rwtimer_.callback(ref, ms, timers);
}

void
server::do_setrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.setrwtimer(ms, flags);
}

void
server::do_resetrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.resetrwtimer(ms, flags);
}

void
server::do_resetrwtimer(unsigned flags)
{
    rwtimer_.resetrwtimer(flags);
}

void
server::do_cancelrwtimer(unsigned flags)
{
    rwtimer_.cancelrwtimer(flags);
}

augas_sock&
server::do_sock()
{
    return conn_.sock();
}

const augas_sock&
server::do_sock() const
{
    return conn_.sock();
}

const sessptr&
server::do_sess() const
{
    return conn_.sess();
}

smartfd
server::do_sfd() const
{
    return conn_.sfd();
}

bool
server::do_accept(const aug_endpoint& ep)
{
    return conn_.accept(ep);
}

void
server::do_connected(const aug_endpoint& ep)
{
    conn_.connected(ep);
}

bool
server::do_process(mplexer& mplexer)
{
    return conn_.process(mplexer);
}

void
server::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    conn_.putsome(mplexer, buf, size);
}

void
server::do_shutdown()
{
    conn_.shutdown();
}

void
server::do_teardown()
{
    conn_.teardown();
}

const endpoint&
server::do_endpoint() const
{
    return conn_.endpoint();
}

connphase
server::do_phase() const
{
    return conn_.phase();
}

server::~server() AUG_NOTHROW
{
}

server::server(const sessptr& sess, void* user, timers& timers,
               const smartfd& sfd, const aug::endpoint& ep)
    : rwtimer_(sess, sock_, timers),
      conn_(sess, sock_, buffer_, rwtimer_, sfd, ep, false)
{
    sock_.sess_ = cptr(*sess);
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
