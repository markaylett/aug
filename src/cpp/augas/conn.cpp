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
conn::do_callback(idref ref, unsigned& ms, aug_timers& timers)
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

conn::~conn() AUG_NOTHROW
{
    try {
        if (open_)
            sess_->close(file_);
    } AUG_PERRINFOCATCH;
}

conn::conn(const sessptr& sess, const smartfd& sfd, augas_id cid,
           void* user, timers& timers)
    : sess_(sess),
      sfd_(sfd),
      rdtimer_(timers, null),
      wrtimer_(timers, null),
      open_(false),
      teardown_(false),
      shutdown_(false)
{
    file_.sess_ = cptr(*sess_);
    file_.id_ = cid;
    file_.user_ = user;
}

void
conn::open(const aug_endpoint& ep)
{
    if (!open_) {
        inetaddr addr(null);
        sess_->openconn(file_, inetntop(getinetaddr(ep, addr)).c_str(),
                        port(ep));
        open_ = true;
    }
}

bool
conn::process(mplexer& mplexer)
{
    unsigned short bits(ioevents(mplexer, sfd_));

    if (bits & AUG_IOEVENTRD) {

        AUG_DEBUG2("handling read event '%d'", sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_DEBUG2("closing connection '%d'", sfd_.get());
            return false;
        }

        // Data has been read: reset read timer.

        if (null != rdtimer_)
            if (!rdtimer_.reset()) // If timer nolonger exists.
                rdtimer_ = null;

        // Notify module of new data.

        data(buf, size);
    }

    if (bits & AUG_IOEVENTWR) {

        bool more(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        if (null != wrtimer_)
            if (!wrtimer_.reset()) // If timer nolonger exists.
                wrtimer_ = null;

        if (!more) {

            // No more (buffered) data to be written.

            setioeventmask(mplexer, sfd_, AUG_IOEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (shutdown_)
                aug::shutdown(sfd_, SHUT_WR);
        }
    }

    return true;
}

void
conn::putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    buffer_.putsome(buf, size);
    setioeventmask(mplexer, sfd_, AUG_IOEVENTRDWR);
}

void
conn::setrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.set(ms, *this);

    if (flags & AUGAS_TIMWR)
        wrtimer_.set(ms, *this);
}

void
conn::shutdown()
{
    shutdown_ = true;
    if (buffer_.empty()) {
        AUG_DEBUG2("shutdown() for id '%d'", sfd_.get());
        aug::shutdown(sfd_, SHUT_WR);
    }
}

void
conn::resetrwtimer(unsigned ms, unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.reset(ms);

    if (flags & AUGAS_TIMWR)
        wrtimer_.reset(ms);
}

void
conn::cancelrwtimer(unsigned flags)
{
    if (flags & AUGAS_TIMRD)
        rdtimer_.cancel();

    if (flags & AUGAS_TIMWR)
        wrtimer_.cancel();
}

void
conn::teardown()
{
    if (!teardown_) {
        teardown_ = true;
        sess_->teardown(file_);
    }
}
