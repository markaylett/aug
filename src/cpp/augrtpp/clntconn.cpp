/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/clntconn.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

void
clntconn::do_timercb(int id, unsigned& ms)
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

augmod_object&
clntconn::do_get()
{
    return conn_->get();
}

const augmod_object&
clntconn::do_get() const
{
    return conn_->get();
}

const sessionptr&
clntconn::do_session() const
{
    return conn_->session();
}

smartfd
clntconn::do_sfd() const
{
    return conn_->sfd();
}

void
clntconn::do_send(const void* buf, size_t len, const timeval& now)
{
    conn_->send(buf, len, now);
}

void
clntconn::do_sendv(const aug_var& var, const timeval& now)
{
    conn_->sendv(var, now);
}

bool
clntconn::do_accepted(const aug_endpoint& ep, const timeval& now)
{
    return conn_->accepted(ep, now);
}

void
clntconn::do_connected(const aug_endpoint& ep, const timeval& now)
{
    conn_->connected(ep, now);
}

bool
clntconn::do_process(unsigned short events, const timeval& now)
{
    if (!conn_->process(events, now))
        return false;

    if (CONNECTED == conn_->state()) {

        // Connection is now established.  If data has been buffered for
        // writing then set the write event-mask.

        if (!buffer_.empty())
            setnbeventmask(conn_->sfd(), AUG_FDEVENTRDWR);

        AUG_DEBUG2("connection now established, assuming new state");

        conn_ = connptr(new aug::connected
                        (conn_->session(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->peername(), true));
    }

    return true;
}

void
clntconn::do_shutdown(unsigned flags, const timeval& now)
{
    conn_->shutdown(flags, now);
}

void
clntconn::do_teardown(const timeval& now)
{
    conn_->teardown(now);
}

bool
clntconn::do_authcert(const char* subject, const char* issuer)
{
    return conn_->authcert(subject, issuer);
}

const endpoint&
clntconn::do_peername() const
{
    return conn_->peername();
}

sockstate
clntconn::do_state() const
{
    return conn_->state();
}

clntconn::~clntconn() AUG_NOTHROW
{
}

clntconn::clntconn(const sessionptr& session, void* user, timers& timers,
                   const char* host, const char* port)
    : rwtimer_(session, sock_, timers),
      conn_(new handshake(session, sock_, buffer_, host, port))
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;

    if (CONNECTED == conn_->state()) {

        AUG_DEBUG2("connection now established, assuming new state");

        conn_ = connptr(new aug::connected
                        (conn_->session(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->peername(), true));
    }
}
