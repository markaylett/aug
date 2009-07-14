/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
    diffms(const aug_timeval& before, const aug_timeval& after)
    {
        aug_timeval diff(after);
        return tvtoms(tvsub(diff, before));
    }

    // Two safety checks designed to deal with misbehaving clients.  Close
    // connection if:

    // 1. Data is pending in write buffer, and a significant amount of time
    // has elapsed waiting for the socket to become writable.

    // 2. The last write was a partial write, and it is estimated that the
    // contents of the buffer cannot be cleared in a timely fashion.

    void
    checkmaxwait(size_t size, const aug_timeval& since,
                 const aug_timeval& now)
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
    checkmaxtime(size_t n, size_t size, const aug_timeval& since,
                 const aug_timeval& now)
    {
        // How long has is taken to perform this write?

        unsigned ms(diffms(since, now));
        if (0 == ms)
            return;

        // What does that equate to per ms?

        unsigned perms(static_cast<unsigned>(n) / ms);
        if (0 == perms)
            return;

        // How long, therefore, will it take to clear buffer?

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

connimpl::~connimpl() AUG_NOTHROW
{
    try {

        // Only accepted connections should be notified of closure.  I.e. if
        // the connection was rejected by session, then do not notify.

        if (accepted_)
            session_.closed(sock_);

    } AUG_PERRINFOCATCH;
}

connimpl::connimpl(session_base& session, mod_handle& sock,
                   buffer& buffer, rwtimer& rwtimer, mod_bool accepted)
    : session_(session),
      sock_(sock),
      buffer_(buffer),
      rwtimer_(rwtimer),
      state_(ESTABLISHED),
      accepted_(accepted)
{
    gettimeofday(getclock(aug_tlx), since_);
}

void
connimpl::send(chanref chan, const void* buf, size_t len,
               const aug_timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setchanwantwr(chan, AUG_TRUE);
    }

    buffer_.append(buf, len);
}

void
connimpl::sendv(chanref chan, blobref blob, const aug_timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setchanwantwr(chan, AUG_TRUE);
    }

    buffer_.append(blob);
}

mod_bool
connimpl::accepted(const string& name, const aug_timeval& now)
{
    return accepted_ = session_.accepted(sock_, name.c_str());
}

void
connimpl::connected(const string& name, const aug_timeval& now)
{
    session_.connected(sock_, name.c_str());
}

mod_bool
connimpl::auth(const char* subject, const char* issuer)
{
    return session_.auth(sock_, subject, issuer);
}

void
connimpl::process(chanref chan, unsigned short events, const aug_timeval& now)
{
    streamptr stream(object_cast<aug_stream>(chan));

    if (events & AUG_MDEVENTRD) {

        AUG_CTXDEBUG2(aug_tlx, "handling read event: id=[%d]",
                      static_cast<int>(sock_.id_));

        try {
            char buf[AUG_BUFSIZE];
            size_t size(read(stream, buf, sizeof(buf)));
            if (0 == size) {

                // Connection closed.

                AUG_CTXDEBUG2(aug_tlx, "closing connection: id=[%d]",
                              static_cast<int>(sock_.id_));
                state_ = CLOSED;
                return;
            }

            // Data has been read: reset read timer.

            rwtimer_.resetrwtimer(MOD_TIMRD);

            // Notify module of new data.

            session_.recv(sock_, buf, size);

        } catch (const block_exception&) {
        }
    }

    if (events & AUG_MDEVENTWR) {

        AUG_CTXDEBUG2(aug_tlx, "handling write event: id=[%d]",
                      static_cast<int>(sock_.id_));

        size_t n(buffer_.writesome(stream));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(MOD_TIMWR);

        if (buffer_.empty()) {

            // No more (buffered) data to be written.

            setchanwantwr(chan, AUG_FALSE);

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
}

void
connimpl::error(const char* desc)
{
    return session_.error(sock_, desc);
}

void
connimpl::shutdown(chanref chan, unsigned flags, const aug_timeval& now)
{
    if (SHUTDOWN <= state_)
        return; // Already shutdown.

    aug_ctxinfo(aug_tlx, "shutting connection: id=[%d], flags=[%u]",
                static_cast<int>(sock_.id_), flags);

    streamptr stream(object_cast<aug_stream>(chan));

    if (flags & MOD_SHUTNOW || null == stream) {

        // Immediate closure or not a stream.

        state_ = CLOSED;

        closechan(chan);

    } else {

        state_ = SHUTDOWN;

        if (buffer_.empty())
            aug::shutdown(stream);
    }
}

void
connimpl::teardown(const aug_timeval& now)
{
    if (state_ < TEARDOWN) {
        state_ = TEARDOWN;
        session_.teardown(sock_);
    }
}

string
connimpl::peername(chanref chan) const
{
    string name;

    char buf[AUG_MAXCHANNAMELEN + 1];
    if (aug_getchanname(chan.get(), buf, sizeof(buf)))
        name = buf;

    return name;
}
