/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/conn.hpp"

#include "augas/exception.hpp"

#include <cassert>

using namespace aug;
using namespace augas;
using namespace std;

rwtimer_base::~rwtimer_base() AUG_NOTHROW
{
}

conn_base::~conn_base() AUG_NOTHROW
{
}

void
rwtimer::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    if (rdtimer_.id() == ref) {
        AUG_DEBUG2("read timer expiry");
        sess_->rdexpire(file_, ms);
    } else if (wrtimer_.id() == ref) {
        AUG_DEBUG2("write timer expiry");
        sess_->wrexpire(file_, ms);
    } else
        assert(0);
}

void
rwtimer::do_setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & AUGAS_TIMWR)
        wrtimer_.set(ms, *this);
}

void
rwtimer::do_resetrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.reset(ms);

    if (flags & AUGAS_TIMWR)
        wrtimer_.reset(ms);
}

void
rwtimer::do_resetrwtimer(unsigned flags)
{
    if (flags & AUGAS_TIMRD && null != rdtimer_)
        if (!rdtimer_.reset()) // If timer nolonger exists.
            rdtimer_ = null;

    if (flags & AUGAS_TIMWR && null != wrtimer_)
        if (!wrtimer_.reset()) // If timer nolonger exists.
            wrtimer_ = null;
}

void
rwtimer::do_cancelrwtimer(unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.cancel();

    if (flags & AUGAS_TIMWR)
        wrtimer_.cancel();
}

rwtimer::~rwtimer() AUG_NOTHROW
{
}

rwtimer::rwtimer(const sessptr& sess, const augas_file& file, timers& timers)
    : sess_(sess),
      file_(file),
      rdtimer_(timers, null),
      wrtimer_(timers, null)
{
}

augas_file&
connected::do_file()
{
    return file_;
}

const augas_file&
connected::do_file() const
{
    return file_;
}

const sessptr&
connected::do_sess() const
{
    return sess_;
}

smartfd
connected::do_sfd() const
{
    return sfd_;
}

bool
connected::do_accept(const aug_endpoint& ep)
{
    inetaddr addr(null);
    return close_ = sess_
        ->accept(file_, inetntop(getinetaddr(ep, addr)).c_str(), port(ep));
}

void
connected::do_connect(const aug_endpoint& ep)
{
    inetaddr addr(null);
    sess_->connect(file_, inetntop(getinetaddr(ep, addr)).c_str(), port(ep));
}

bool
connected::do_process(mplexer& mplexer)
{
    unsigned short bits(ioevents(mplexer, sfd_));

    if (bits & AUG_IOEVENTRD) {

        AUG_DEBUG2("handling read event: id=[%d], fd=[%d]", file_.id_,
                   sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_DEBUG2("closing connection: id=[%d], fd=[%d]", file_.id_,
                       sfd_.get());
            phase_ = CLOSED;
            return true;
        }

        // Data has been read: reset read timer.

        rwtimer_.resetrwtimer(AUGAS_TIMRD);

        // Notify module of new data.

        sess_->data(file_, buf, size);
    }

    if (bits & AUG_IOEVENTWR) {

        bool more(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(AUGAS_TIMWR);

        if (!more) {

            // No more (buffered) data to be written.

            setioeventmask(mplexer, sfd_, AUG_IOEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (SHUTDOWN <= phase_)
                aug::shutdown(sfd_, SHUT_WR);
        }
    }

    return false;
}

void
connected::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    buffer_.putsome(buf, size);
    setioeventmask(mplexer, sfd_, AUG_IOEVENTRDWR);
}

void
connected::do_shutdown()
{
    if (phase_ < SHUTDOWN) {
        phase_ = SHUTDOWN;
        if (buffer_.empty()) {
            AUG_DEBUG2("shutdown(): id=[%d], fd=[%d]", file_.id_, sfd_.get());
            aug::shutdown(sfd_, SHUT_WR);
        }
    }
}

void
connected::do_teardown()
{
    if (phase_ < TEARDOWN) {
        phase_ = TEARDOWN;
        sess_->teardown(file_);
    }
}

const endpoint&
connected::do_endpoint() const
{
    return endpoint_;
}

connphase
connected::do_phase() const
{
    return phase_;
}

connected::~connected() AUG_NOTHROW
{
    try {
        if (close_)
            sess_->close(file_);
    } AUG_PERRINFOCATCH;
}

connected::connected(const sessptr& sess, augas_file& file, rwtimer& rwtimer,
                     const smartfd& sfd, const aug::endpoint& ep, bool close)
    : sess_(sess),
      file_(file),
      rwtimer_(rwtimer),
      endpoint_(ep),
      sfd_(sfd),
      phase_(CONNECTED),
      close_(close)
{
}

augas_file&
connecting::do_file()
{
    return file_;
}

const augas_file&
connecting::do_file() const
{
    return file_;
}

const sessptr&
connecting::do_sess() const
{
    return sess_;
}

smartfd
connecting::do_sfd() const
{
    return sfd_;
}

bool
connecting::do_accept(const aug_endpoint& ep)
{
    return false;
}

void
connecting::do_connect(const aug_endpoint& ep)
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", file_.id_);
}

bool
connecting::do_process(mplexer& mplexer)
{
    try {

        pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
        sfd_ = xy.first;
        if (xy.second) {
            phase_ = CONNECTED;
            return true;
        }

    } catch (const errinfo_error& e) {

        perrinfo(e, "connection failed");
        phase_ = CLOSED;
        return true;
    }

    return false;
}

void
connecting::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", file_.id_);
}

void
connecting::do_shutdown()
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", file_.id_);
}

void
connecting::do_teardown()
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", file_.id_);
}

const endpoint&
connecting::do_endpoint() const
{
    return endpoint_;
}

connphase
connecting::do_phase() const
{
    return phase_;
}

connecting::~connecting() AUG_NOTHROW
{
    try {
        if (CLOSED == phase_)
            sess_->close(file_);
    } AUG_PERRINFOCATCH;
}

connecting::connecting(const sessptr& sess, augas_file& file,
                       const char* host, const char* serv)
    : sess_(sess),
      file_(file),
      connector_(host, serv),
      endpoint_(null),
      sfd_(null),
      phase_(CLOSED)
{
    pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
    sfd_ = xy.first;
    phase_ = xy.second ? CONNECTED : CONNECTING;
}

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

augas_file&
client::do_file()
{
    return conn_->file();
}

const augas_file&
client::do_file() const
{
    return conn_->file();
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
client::do_connect(const aug_endpoint& ep)
{
    conn_->connect(ep);
}

bool
client::do_process(mplexer& mplexer)
{
    if (!conn_->process(mplexer))
        return false;

    if (CONNECTED == conn_->phase()) {

        AUG_DEBUG2("client is now connected, assuming new state");

        conn_ = connptr(new connected(conn_->sess(), file_, rwtimer_,
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

client::client(const sessptr& sess, augas_id cid, void* user, timers& timers,
               const char* host, const char* serv)
    : rwtimer_(sess, file_, timers),
      conn_(new connecting(sess, file_, host, serv))
{
    file_.sess_ = cptr(*sess);
    file_.id_ = cid;
    file_.user_ = user;

    if (CONNECTED == conn_->phase()) {

        AUG_DEBUG2("client is now connected, assuming new state");

        conn_ = connptr(new connected(conn_->sess(), file_, rwtimer_,
                                      conn_->sfd(), conn_->endpoint(), true));
    }
}

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

augas_file&
server::do_file()
{
    return conn_.file();
}

const augas_file&
server::do_file() const
{
    return conn_.file();
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
server::do_connect(const aug_endpoint& ep)
{
    conn_.connect(ep);
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

server::server(const sessptr& sess, augas_id cid, void* user, timers& timers,
               const smartfd& sfd, const aug::endpoint& ep)
    : rwtimer_(sess, file_, timers),
      conn_(sess, file_, rwtimer_, sfd, ep, false)
{
    file_.sess_ = cptr(*sess);
    file_.id_ = cid;
    file_.user_ = user;
}
