/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
#include "augaspp/conn.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augaspp/buffer.hpp"

#include "augmodpp.hpp"

#include <cassert>

// Maximum time that can be spent waiting for writability.

#if !defined(AUG_WRMAXWAIT)
# define AUG_WRMAXWAIT (5 * 1000)
#endif // !AUG_WRMAXWAIT

// Maximum time that can be spent writing buffer contents.

#if !defined(AUG_WRMAXTIME)
# define AUG_WRMAXTIME (30 * 1000)
#endif // !AUG_WRMAXTIME

// Size threshold at which AUG_WRMAXTIME checks begin.

#if !defined(AUG_WRCHECKAT)
# define AUG_WRCHECKAT (8 * 1024)
#endif // !AUG_WRCHECKAT

using namespace aug;
using namespace std;

namespace {

    unsigned
    diffms(const timeval& before, const timeval& after)
    {
        timeval diff(after);
        return tvtoms(tvsub(diff, before));
    }

    // Two safety checks designed to deal with misbehaving clients.  Close
    // connection if:

    // 1. Data is pending in write buffer, and a significant amount of time
    // has elapsed waiting for the socket to become writable.

    // 2. The last write was a partial write, and it is estimated that the
    // contents of the buffer cannot be cleared in a timely fashion.

    void
    checkmaxwait(size_t size, const timeval& since, const timeval& now)
    {
        unsigned ms(diffms(since, now));

        if (AUG_WRMAXWAIT < ms)
            throw aug_error(__FILE__, __LINE__, AUG_ETIMEOUT,
                            "write wait exceeded: pending=[%u], since=[%u]",
                            static_cast<unsigned>(size), ms);

        if (AUG_WRMAXWAIT * 2 < ms * 3)
            aug_ctxwarn(aug_tlx, "write latency: pending=[%u], since=[%u]",
                        static_cast<unsigned>(size), ms);
    }

    void
    checkmaxtime(size_t n, size_t size, const timeval& since,
                 const timeval& now)
    {
        // How long has is taken to perform this write.

        unsigned ms(diffms(since, now));
        if (0 == ms)
            return;

        // What does that equate to per ms.

        unsigned perms(static_cast<unsigned>(n) / ms);
        if (0 == perms)
            return;

        // How long, therefore, will it take to clear buffer.

        unsigned estms(static_cast<unsigned>(size) / perms);
        if (0 == estms)
            return;

        if (AUG_WRMAXTIME < estms)
            throw aug_error(__FILE__, __LINE__, AUG_ETIMEOUT,
                            "write time exceeded: perms=[%u], estms=[%u]",
                            perms, estms);

        if (AUG_WRMAXTIME * 2 < estms * 3)
            aug_ctxwarn(aug_tlx, "write time: perms=[%u], estms=[%u]", perms,
                        estms);
    }
}

conn_base::~conn_base() AUG_NOTHROW
{
}

mod_handle&
connected::do_get()
{
    return sock_;
}

const mod_handle&
connected::do_get() const
{
    return sock_;
}

const sessionptr&
connected::do_session() const
{
    return session_;
}

chanptr
connected::do_chan() const
{
    return chan_;
}

void
connected::do_send(const void* buf, size_t len, const timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setchanmask(chan_, AUG_FDEVENTRDWR);
    }

    buffer_.append(buf, len);
}

void
connected::do_sendv(blobref ref, const timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setchanmask(chan_, AUG_FDEVENTRDWR);
    }

    buffer_.append(ref);
}

bool
connected::do_accepted(const string& name, const timeval& now)
{
    return close_ = session_->accepted(sock_, name.c_str());
}

void
connected::do_connected(const string& name, const timeval& now)
{
    session_->connected(sock_, name.c_str());
}

bool
connected::do_process(obref<aug_stream> stream, unsigned short events,
                      const timeval& now)
{
    chan_ = object_cast<aug_chan>(stream);

    if (events & AUG_FDEVENTRD) {

        AUG_CTXDEBUG2(aug_tlx, "handling read event: id=[%u]", sock_.id_);

        char buf[4096];
        size_t size(read(stream, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_CTXDEBUG2(aug_tlx, "closing connection: id=[%u]", sock_.id_);
            state_ = CLOSED;
            return true;
        }

        // Data has been read: reset read timer.

        rwtimer_.resetrwtimer(MOD_TIMRD);

        // Notify module of new data.

        session_->data(sock_, buf, size);
    }

    if (events & AUG_FDEVENTWR) {

        AUG_CTXDEBUG2(aug_tlx, "handling write event: id=[%u]", sock_.id_);

        size_t n(buffer_.writesome(stream));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(MOD_TIMWR);

        if (buffer_.empty()) {

            // No more (buffered) data to be written.

            setchanmask(chan_, AUG_FDEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (SHUTDOWN <= state_)
                aug::shutdown(stream);

        } else {

            AUG_CTXDEBUG2(aug_tlx,
                          "partial write: written=[%u], remaining=[%u]",
                          static_cast<unsigned>(n),
                          static_cast<unsigned>(buffer_.size()));

            // If size threshold has been exceeded, check if buffer can be
            // cleared in acceptable timeframe.

            if (AUG_WRCHECKAT < buffer_.size())
                checkmaxtime(n, buffer_.size(), since_, now);

            // Partial write.  Reset wait since time.

            since_ = now;
        }

    } else {

        // Not writable.  Check max wait if write buffer is not empty.

        if (!buffer_.empty())
            checkmaxwait(buffer_.size(), since_, now);
    }

    return false;
}

void
connected::do_shutdown(unsigned flags, const timeval& now)
{
    if (state_ < SHUTDOWN) {
        state_ = SHUTDOWN;
        if (buffer_.empty() || flags & MOD_SHUTNOW) {
            aug_ctxinfo(aug_tlx,
                        "shutting connection: id=[%u], flags=[%u]",
                        sock_.id_, flags);
            streamptr stream(object_cast<aug_stream>(chan_));
            // FIXME: should be avoided if state_ set correctly.
			if (null != stream)
				aug::shutdown(stream);
        }
    }
}

void
connected::do_teardown(const timeval& now)
{
    if (state_ < TEARDOWN) {
        state_ = TEARDOWN;
        session_->teardown(sock_);
    }
}

bool
connected::do_authcert(const char* subject, const char* issuer)
{
    return session_->authcert(sock_, subject, issuer);
}

string
connected::do_peername() const
{
    string name;

    char buf[AUG_MAXCHANNAMELEN + 1];
    if (aug_getchanname(chan_.get(), buf, sizeof(buf)))
        name = buf;

    return name;
}

sockstate
connected::do_state() const
{
    return state_;
}

connected::~connected() AUG_NOTHROW
{
    try {
        if (close_)
            session_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

connected::connected(const sessionptr& session, mod_handle& sock,
                     buffer& buffer, rwtimer& rwtimer,
                     const chanptr& chan, bool close)
    : session_(session),
      sock_(sock),
      buffer_(buffer),
      rwtimer_(rwtimer),
      chan_(chan),
      state_(ESTABLISHED),
      close_(close)
{
    gettimeofday(since_);
}
