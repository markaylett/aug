/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/clntconn.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id:$");

using namespace aug;
using namespace augas;
using namespace std;

void
clntconn::do_timercb(int id, unsigned& ms, aug_timers& timers)
{
    rwtimer_.timercb(id, ms, timers);
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

augas_object&
clntconn::do_object()
{
    return conn_->object();
}

const augas_object&
clntconn::do_object() const
{
    return conn_->object();
}

const servptr&
clntconn::do_serv() const
{
    return conn_->serv();
}

smartfd
clntconn::do_sfd() const
{
    return conn_->sfd();
}

bool
clntconn::do_accept(const aug_endpoint& ep)
{
    return conn_->accept(ep);
}

void
clntconn::do_append(aug::mplexer& mplexer, const aug_var& var)
{
    conn_->append(mplexer, var);
}

void
clntconn::do_append(aug::mplexer& mplexer, const void* buf, size_t len)
{
    conn_->append(mplexer, buf, len);
}

void
clntconn::do_connected(const aug_endpoint& ep)
{
    conn_->connected(ep);
}

bool
clntconn::do_process(mplexer& mplexer)
{
    if (!conn_->process(mplexer))
        return false;

    if (ESTABLISHED == conn_->phase()) {

        // Connection is now established.  If data has been buffered for
        // writing then set the write event-mask.

        if (!buffer_.empty())
            setioeventmask(mplexer, conn_->sfd(), AUG_IOEVENTRDWR);

        AUG_DEBUG2("connection is now established, assuming new state");

        conn_ = connptr(new augas::established
                        (conn_->serv(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->endpoint(), true));
    }

    return true;
}

void
clntconn::do_shutdown()
{
    conn_->shutdown();
}

void
clntconn::do_teardown()
{
    conn_->teardown();
}

const endpoint&
clntconn::do_endpoint() const
{
    return conn_->endpoint();
}

connphase
clntconn::do_phase() const
{
    return conn_->phase();
}

clntconn::~clntconn() AUG_NOTHROW
{
}

clntconn::clntconn(const servptr& serv, void* user, timers& timers,
               const char* host, const char* port)
    : rwtimer_(serv, sock_, timers),
      conn_(new connecting(serv, sock_, buffer_, host, port))
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;

    if (ESTABLISHED == conn_->phase()) {

        AUG_DEBUG2("clntconn is now established, assuming new state");

        conn_ = connptr(new augas::established
                        (conn_->serv(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->endpoint(), true));
    }
}
