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
conn::do_file()
{
    return file_;
}

int
conn::do_fd() const
{
    return sfd_.get();
}

const augas_file&
conn::do_file() const
{
    return file_;
}

const sessptr&
conn::do_sess() const
{
    return sess_;
}

void
conn::do_accept(const aug_endpoint& ep)
{
    if (!open_) {
        inetaddr addr(null);
        sess_->accept(file_, inetntop(getinetaddr(ep, addr)).c_str(),
                      port(ep));
        open_ = true;
    }
}

void
conn::do_connect(const aug_endpoint& ep)
{
    if (!open_) {
        inetaddr addr(null);
        sess_->connect(file_, inetntop(getinetaddr(ep, addr)).c_str(),
                       port(ep));
        open_ = true;
    }
}

connstate
conn::do_process(mplexer& mplexer)
{
    unsigned short bits(ioevents(mplexer, sfd_));

    if (bits & AUG_IOEVENTRD) {

        AUG_DEBUG2("handling read event '%d'", sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_DEBUG2("closing connection '%d'", sfd_.get());
            return CLOSED;
        }

        // Data has been read: reset read timer.

        rwtimer_.resetrwtimer(AUGAS_TIMRD);

        // Notify module of new data.

        data(buf, size);
    }

    if (bits & AUG_IOEVENTWR) {

        bool more(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(AUGAS_TIMWR);

        if (!more) {

            // No more (buffered) data to be written.

            setioeventmask(mplexer, sfd_, AUG_IOEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (shutdown_)
                aug::shutdown(sfd_, SHUT_WR);
        }
    }

    return CONNECTED;
}

void
conn::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    buffer_.putsome(buf, size);
    setioeventmask(mplexer, sfd_, AUG_IOEVENTRDWR);
}

void
conn::do_shutdown()
{
    shutdown_ = true;
    if (buffer_.empty()) {
        AUG_DEBUG2("shutdown() for id '%d'", sfd_.get());
        aug::shutdown(sfd_, SHUT_WR);
    }
}

void
conn::do_teardown()
{
    if (!teardown_) {
        teardown_ = true;
        sess_->teardown(file_);
    }
}

void
conn::do_data(const char* buf, size_t size) const
{
    sess_->data(file_, buf, size);
}

bool
conn::do_isopen() const
{
    return open_;
}

bool
conn::do_isshutdown() const
{
    return shutdown_;
}

conn::~conn() AUG_NOTHROW
{
    try {
        if (open_)
            sess_->close(file_);
    } AUG_PERRINFOCATCH;
}

conn::conn(const sessptr& sess, augas_file& file, rwtimer& rwtimer,
           const smartfd& sfd)
    : sess_(sess),
      file_(file),
      rwtimer_(rwtimer),
      sfd_(sfd),
      open_(false),
      teardown_(false),
      shutdown_(false)
{
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
    return conn_.file();
}

int
client::do_fd() const
{
    return conn_.fd();
}

const augas_file&
client::do_file() const
{
    return conn_.file();
}

const sessptr&
client::do_sess() const
{
    return conn_.sess();
}

void
client::do_accept(const aug_endpoint& ep)
{
    conn_.accept(ep);
}

void
client::do_connect(const aug_endpoint& ep)
{
    conn_.connect(ep);
}

connstate
client::do_process(mplexer& mplexer)
{
    return conn_.process(mplexer);
}

void
client::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    conn_.putsome(mplexer, buf, size);
}

void
client::do_shutdown()
{
    conn_.shutdown();
}

void
client::do_teardown()
{
    conn_.teardown();
}

void
client::do_data(const char* buf, size_t size) const
{
    conn_.data(buf, size);
}

bool
client::do_isopen() const
{
    return conn_.isopen();
}

bool
client::do_isshutdown() const
{
    return conn_.isshutdown();
}

client::~client() AUG_NOTHROW
{

}

client::client(const sessptr& sess, augas_id cid, void* user, timers& timers,
               const smartfd& sfd)
    : rwtimer_(sess, file_, timers),
      conn_(sess, file_, rwtimer_, sfd)
{
    file_.sess_ = cptr(*sess);
    file_.id_ = cid;
    file_.user_ = user;
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

int
server::do_fd() const
{
    return conn_.fd();
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

void
server::do_accept(const aug_endpoint& ep)
{
    conn_.accept(ep);
}

void
server::do_connect(const aug_endpoint& ep)
{
    conn_.connect(ep);
}

connstate
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

void
server::do_data(const char* buf, size_t size) const
{
    conn_.data(buf, size);
}

bool
server::do_isopen() const
{
    return conn_.isopen();
}

bool
server::do_isshutdown() const
{
    return conn_.isshutdown();
}

server::~server() AUG_NOTHROW
{

}

server::server(const sessptr& sess, augas_id cid, void* user, timers& timers,
               const smartfd& sfd)
    : rwtimer_(sess, file_, timers),
      conn_(sess, file_, rwtimer_, sfd)
{
    file_.sess_ = cptr(*sess);
    file_.id_ = cid;
    file_.user_ = user;
}
