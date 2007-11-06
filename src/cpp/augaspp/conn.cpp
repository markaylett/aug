/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/conn.hpp"
#include "augsys/defs.h"

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
            throw local_error(__FILE__, __LINE__, AUG_ETIMEOUT,
                              "write wait exceeded: pending=[%u], since=[%u]",
                              static_cast<unsigned>(size), ms);

        if (AUG_WRMAXWAIT * 2 < ms * 3)
            aug_warn("write latency: pending=[%u], since=[%u]",
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
            throw local_error(__FILE__, __LINE__, AUG_ETIMEOUT,
                              "write time exceeded: perms=[%u], estms=[%u]",
                              perms, estms);

        if (AUG_WRMAXTIME * 2 < estms * 3)
            aug_warn("write time: perms=[%u], estms=[%u]", perms, estms);
    }
}

conn_base::~conn_base() AUG_NOTHROW
{
}

augmod_object&
connected::do_get()
{
    return sock_;
}

const augmod_object&
connected::do_get() const
{
    return sock_;
}

const sessionptr&
connected::do_session() const
{
    return session_;
}

smartfd
connected::do_sfd() const
{
    return sfd_;
}

void
connected::do_send(const void* buf, size_t len, const timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setnbeventmask(sfd_, AUG_FDEVENTRDWR);
    }

    buffer_.append(buf, len);
}

void
connected::do_sendv(const aug_var& var, const timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was first queued for write.

        since_ = now;
        setnbeventmask(sfd_, AUG_FDEVENTRDWR);
    }

    buffer_.append(var);
}

bool
connected::do_accepted(const aug_endpoint& ep, const timeval& now)
{
    inetaddr addr(null);
    return close_ = session_
        ->accepted(sock_, inetntop(getinetaddr(ep, addr)).c_str(), port(ep));
}

void
connected::do_connected(const aug_endpoint& ep, const timeval& now)
{
    inetaddr addr(null);
    session_->connected(sock_, inetntop(getinetaddr(ep, addr)).c_str(),
                        port(ep));
}

bool
connected::do_process(unsigned short events, const timeval& now)
{
    if (events & AUG_FDEVENTRD) {

        AUG_DEBUG2("handling read event: id=[%d], fd=[%d]", sock_.id_,
                   sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_DEBUG2("closing connection: id=[%d], fd=[%d]", sock_.id_,
                       sfd_.get());
            state_ = CLOSED;
            return true;
        }

        // Data has been read: reset read timer.

        rwtimer_.resetrwtimer(AUGMOD_TIMRD);

        // Notify module of new data.

        session_->data(sock_, buf, size);
    }

    if (events & AUG_FDEVENTWR) {

        AUG_DEBUG2("handling write event: id=[%d], fd=[%d]", sock_.id_,
                   sfd_.get());

        size_t n(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(AUGMOD_TIMWR);

        if (buffer_.empty()) {

            // No more (buffered) data to be written.

            setnbeventmask(sfd_, AUG_FDEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (SHUTDOWN <= state_)
                shutdownnbfile(sfd_);

        } else {

            AUG_DEBUG2("partial write: written=[%u], remaining=[%u]",
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
        if (buffer_.empty() || flags & AUGMOD_SHUTNOW) {
            aug_info("shutting connection: id=[%d], fd=[%d], flags=[%u]",
                     sock_.id_, sfd_.get(), flags);
            shutdownnbfile(sfd_);
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

const endpoint&
connected::do_peername() const
{
    return endpoint_;
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

connected::connected(const sessionptr& session, augmod_object& sock,
                     buffer& buffer, rwtimer& rwtimer, const smartfd& sfd,
                     const endpoint& ep, bool close)
    : session_(session),
      sock_(sock),
      buffer_(buffer),
      rwtimer_(rwtimer),
      sfd_(sfd),
      endpoint_(ep),
      state_(CONNECTED),
      close_(close)
{
    gettimeofday(since_);
}

augmod_object&
handshake::do_get()
{
    return sock_;
}

const augmod_object&
handshake::do_get() const
{
    return sock_;
}

const sessionptr&
handshake::do_session() const
{
    return session_;
}

smartfd
handshake::do_sfd() const
{
    return sfd_;
}

void
handshake::do_send(const void* buf, size_t len, const timeval& now)
{
    buffer_.append(buf, len);
}

void
handshake::do_sendv(const aug_var& var, const timeval& now)
{
    buffer_.append(var);
}

bool
handshake::do_accepted(const aug_endpoint& ep, const timeval& now)
{
    return false;
}

void
handshake::do_connected(const aug_endpoint& ep, const timeval& now)
{
    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

bool
handshake::do_process(unsigned short events, const timeval& now)
{
    try {

        pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
        sfd_ = xy.first;

        // Check to see if connection was established.

        if (xy.second) {
            state_ = CONNECTED;
            return true;
        }

        // Otherwise, try next endpoint associated with address.

    } catch (const errinfo_error& e) {

        perrinfo(e, "connection failed");
        state_ = CLOSED;
        return true;
    }

    return false;
}

void
handshake::do_shutdown(unsigned flags, const timeval& now)
{
    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

void
handshake::do_teardown(const timeval& now)
{
    // TODO: call shutdown() here?

    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

bool
handshake::do_authcert(const char* subject, const char* issuer)
{
    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

const endpoint&
handshake::do_peername() const
{
    return endpoint_;
}

sockstate
handshake::do_state() const
{
    return state_;
}

handshake::~handshake() AUG_NOTHROW
{
    try {
        if (CLOSED == state_)
            session_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

handshake::handshake(const sessionptr& session, augmod_object& sock,
                     buffer& buffer, const char* host, const char* port)
    : session_(session),
      sock_(sock),
      buffer_(buffer),
      connector_(host, port),
      sfd_(null),
      endpoint_(null),
      state_(CLOSED)
{
    pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
    sfd_ = xy.first;
    state_ = xy.second ? CONNECTED : HANDSHAKE;
}
