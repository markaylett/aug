/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/conn.hpp"

#include "augas/exception.hpp"

#include <cassert>

using namespace aug;
using namespace augas;
using namespace std;

void
conn::do_callback(idref ref, unsigned& ms, aug_timers& timers)
{
    if (rdtimer_ == ref) {
        aug_debug2("read timer expiry");
        sess_->rdexpire(conn_, ms);
    } else if (wrtimer_ == ref) {
        aug_debug2("write timer expiry");
        sess_->wrexpire(conn_, ms);
    } else
        assert(0);
}

conn::~conn() AUG_NOTHROW
{
    try {
        if (open_)
            sess_->closeconn(conn_);
    } AUG_PERRINFOCATCH;
}

conn::conn(const sessptr& sess, const smartfd& sfd, augas_id cid,
           timers& timers)
    : sess_(sess),
      sfd_(sfd),
      rdtimer_(timers, null),
      wrtimer_(timers, null),
      open_(false),
      shutdown_(false)
{
    conn_.sess_ = cptr(*sess_);
    conn_.id_ = cid;
    conn_.user_ = 0;
}

void
conn::open(const aug_endpoint& ep)
{
    if (!open_) {
        inetaddr addr(null);
        sess_->openconn(conn_, inetntop(getinetaddr(ep, addr)).c_str(),
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

            aug_debug2("closing connection '%d'", sfd_.get());
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
conn::shutdown()
{
    shutdown_ = true;
    if (buffer_.empty())
        aug::shutdown(sfd_, SHUT_WR);
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
