/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/client.hpp"

using namespace aug;
using namespace augas;
using namespace std;

void
client::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    rwtimer_.callback(ref, ms, timers);
}

void
client::do_setrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.setrwtimer(ms, flags);
}

void
client::do_resetrwtimer(unsigned ms, unsigned flags)
{
    rwtimer_.resetrwtimer(ms, flags);
}

void
client::do_resetrwtimer(unsigned flags)
{
    rwtimer_.resetrwtimer(flags);
}

void
client::do_cancelrwtimer(unsigned flags)
{
    rwtimer_.cancelrwtimer(flags);
}

augas_sock&
client::do_sock()
{
    return conn_->sock();
}

const augas_sock&
client::do_sock() const
{
    return conn_->sock();
}

const sessptr&
client::do_sess() const
{
    return conn_->sess();
}

smartfd
client::do_sfd() const
{
    return conn_->sfd();
}

bool
client::do_accept(const aug_endpoint& ep)
{
    return conn_->accept(ep);
}

void
client::do_connected(const aug_endpoint& ep)
{
    conn_->connected(ep);
}

bool
client::do_process(mplexer& mplexer)
{
    if (!conn_->process(mplexer))
        return false;

    if (ESTABLISHED == conn_->phase()) {

        if (!buffer_.empty())
            setioeventmask(mplexer, conn_->sfd(), AUG_IOEVENTRDWR);

        AUG_DEBUG2("client is now established, assuming new state");

        conn_ = connptr(new augas::established
                        (conn_->sess(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->endpoint(), true));
    }

    return true;
}

void
client::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    conn_->putsome(mplexer, buf, size);
}

void
client::do_shutdown()
{
    conn_->shutdown();
}

void
client::do_teardown()
{
    conn_->teardown();
}

const endpoint&
client::do_endpoint() const
{
    return conn_->endpoint();
}

connphase
client::do_phase() const
{
    return conn_->phase();
}

client::~client() AUG_NOTHROW
{
}

client::client(const sessptr& sess, void* user, timers& timers,
               const char* host, const char* serv)
    : rwtimer_(sess, sock_, timers),
      conn_(new connecting(sess, sock_, buffer_, host, serv))
{
    sock_.sess_ = cptr(*sess);
    sock_.id_ = aug_nextid();
    sock_.user_ = user;

    if (ESTABLISHED == conn_->phase()) {

        AUG_DEBUG2("client is now established, assuming new state");

        conn_ = connptr(new augas::established
                        (conn_->sess(), sock_, buffer_, rwtimer_,
                         conn_->sfd(), conn_->endpoint(), true));
    }
}
