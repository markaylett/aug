/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/conn.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrtpp/buffer.hpp"

#include "augrtpp.hpp"

#include <cassert>

// Default to a one second write timeout.

#if !defined(AUG_WRTIMEOUT)
# define AUG_WRTIMEOUT 1000
#endif // !AUG_WRTIMEOUT

using namespace aug;
using namespace std;


namespace {

    void
    checklatency(const timeval& since, const timeval& now)
    {
        timeval diff(now);
        unsigned ms(tvtoms(tvsub(diff, since)));
#if !defined(NDEBUG)
        if (0 < ms)
            aug_debug2("write latency: ms=[%u]", ms);
#endif // !NDEBUG
        if (AUG_WRTIMEOUT < ms)
            throw local_error(__FILE__, __LINE__, AUG_ETIMEOUT,
                              "excessive write latency: ms=[%u]", ms);
    }
}

conn_base::~conn_base() AUG_NOTHROW
{
}

augrt_object&
connected::do_get()
{
    return sock_;
}

const augrt_object&
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

        // Set timestamp to record when data was queued for write.

        since_ = now;
        buffer_.append(buf, len);
        setnbeventmask(sfd_, AUG_FDEVENTRDWR);

    } else {

        // Perform latency check before appending to buffer.

        checklatency(since_, now);
        buffer_.append(buf, len);
    }
}

void
connected::do_sendv(const aug_var& var, const timeval& now)
{
    if (buffer_.empty()) {

        // Set timestamp to record when data was queued for write.

        since_ = now;
        buffer_.append(var);
        setnbeventmask(sfd_, AUG_FDEVENTRDWR);

    } else {

        // Perform latency check before appending to buffer.

        checklatency(since_, now);
        buffer_.append(var);
    }
}

bool
connected::do_accepted(const aug_endpoint& ep)
{
    inetaddr addr(null);
    return close_ = session_
        ->accepted(sock_, inetntop(getinetaddr(ep, addr)).c_str(), port(ep));
}

void
connected::do_connected(const aug_endpoint& ep)
{
    inetaddr addr(null);
    session_->connected(sock_, inetntop(getinetaddr(ep, addr)).c_str(),
                        port(ep));
}

bool
connected::do_process(unsigned short events)
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

        rwtimer_.resetrwtimer(AUGRT_TIMRD);

        // Notify module of new data.

        session_->data(sock_, buf, size);
    }

    if (events & AUG_FDEVENTWR) {

        AUG_DEBUG2("handling write event: id=[%d], fd=[%d]", sock_.id_,
                   sfd_.get());

        bool done(buffer_.writesome(sfd_));

        // Data has been written: reset write timer.

        rwtimer_.resetrwtimer(AUGRT_TIMWR);

        if (done) {

            // No more (buffered) data to be written.

            setnbeventmask(sfd_, AUG_FDEVENTRD);

            // If flagged for shutdown, send FIN and disable writes.

            if (SHUTDOWN <= state_)
                shutdownnbfile(sfd_);
        }
    }

    return false;
}

void
connected::do_shutdown(unsigned flags)
{
    if (state_ < SHUTDOWN) {
        state_ = SHUTDOWN;
        if (buffer_.empty() || flags & AUGRT_SHUTNOW) {
            aug_info("shutting connection: id=[%d], fd=[%d], flags=[%u]",
                     sock_.id_, sfd_.get(), flags);
            shutdownnbfile(sfd_);
        }
    }
}

void
connected::do_teardown()
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

connected::connected(const sessionptr& session, augrt_object& sock,
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

augrt_object&
handshake::do_get()
{
    return sock_;
}

const augrt_object&
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
handshake::do_accepted(const aug_endpoint& ep)
{
    return false;
}

void
handshake::do_connected(const aug_endpoint& ep)
{
    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

bool
handshake::do_process(unsigned short events)
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
handshake::do_shutdown(unsigned flags)
{
    throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                      AUG_MSG("handshake in progress: id=[%d]"), sock_.id_);
}

void
handshake::do_teardown()
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

handshake::handshake(const sessionptr& session, augrt_object& sock,
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
