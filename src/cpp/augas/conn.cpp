/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/conn.hpp"

#include "augas/buffer.hpp"
#include "augas/exception.hpp"

#include <cassert>

using namespace aug;
using namespace augas;
using namespace std;

conn_base::~conn_base() AUG_NOTHROW
{
}

augas_object&
established::do_object()
{
    return sock_;
}

const augas_object&
established::do_object() const
{
    return sock_;
}

const sessptr&
established::do_sess() const
{
    return sess_;
}

smartfd
established::do_sfd() const
{
    return sfd_;
}

bool
established::do_accept(const aug_endpoint& ep)
{
    inetaddr addr(null);
    return close_ = sess_
        ->accept(sock_, inetntop(getinetaddr(ep, addr)).c_str(), port(ep));
}

void
established::do_connected(const aug_endpoint& ep)
{
    inetaddr addr(null);
    sess_->connected(sock_, inetntop(getinetaddr(ep, addr)).c_str(),
                     port(ep));
}

bool
established::do_process(mplexer& mplexer)
{
    unsigned short bits(ioevents(mplexer, sfd_));

    if (bits & AUG_IOEVENTRD) {

        AUG_DEBUG2("handling read event: id=[%d], fd=[%d]", sock_.id_,
                   sfd_.get());

        char buf[4096];
        size_t size(aug::read(sfd_, buf, sizeof(buf)));
        if (0 == size) {

            // Connection closed.

            AUG_DEBUG2("closing connection: id=[%d], fd=[%d]", sock_.id_,
                       sfd_.get());
            phase_ = CLOSED;
            return true;
        }

        // Data has been read: reset read timer.

        rwtimer_.resetrwtimer(AUGAS_TIMRD);

        // Notify module of new data.

        sess_->data(sock_, buf, size);
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
established::do_putsome(aug::mplexer& mplexer, const void* buf, size_t size)
{
    buffer_.putsome(buf, size);
    setioeventmask(mplexer, sfd_, AUG_IOEVENTRDWR);
}

void
established::do_shutdown()
{
    if (phase_ < SHUTDOWN) {
        phase_ = SHUTDOWN;
        if (buffer_.empty()) {
            AUG_DEBUG2("shutdown(): id=[%d], fd=[%d]", sock_.id_, sfd_.get());
            aug::shutdown(sfd_, SHUT_WR);
        }
    }
}

void
established::do_teardown()
{
    if (phase_ < TEARDOWN) {
        phase_ = TEARDOWN;
        sess_->teardown(sock_);
    }
}

const endpoint&
established::do_endpoint() const
{
    return endpoint_;
}

connphase
established::do_phase() const
{
    return phase_;
}

established::~established() AUG_NOTHROW
{
    try {
        if (close_)
            sess_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

established::established(const sessptr& sess, augas_object& sock,
                         buffer& buffer, rwtimer& rwtimer, const smartfd& sfd,
                         const aug::endpoint& ep, bool close)
    : sess_(sess),
      sock_(sock),
      buffer_(buffer),
      rwtimer_(rwtimer),
      sfd_(sfd),
      endpoint_(ep),
      phase_(ESTABLISHED),
      close_(close)
{
}

augas_object&
connecting::do_object()
{
    return sock_;
}

const augas_object&
connecting::do_object() const
{
    return sock_;
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
connecting::do_connected(const aug_endpoint& ep)
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", sock_.id_);
}

bool
connecting::do_process(mplexer& mplexer)
{
    try {

        pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
        sfd_ = xy.first;
        if (xy.second) {
            phase_ = ESTABLISHED;
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
    buffer_.putsome(buf, size);
}

void
connecting::do_shutdown()
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", sock_.id_);
}

void
connecting::do_teardown()
{
    throw error(__FILE__, __LINE__, ESTATE,
                "connection not established: id=[%d]", sock_.id_);
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
            sess_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

connecting::connecting(const sessptr& sess, augas_object& sock,
                       buffer& buffer, const char* host, const char* serv)
    : sess_(sess),
      sock_(sock),
      buffer_(buffer),
      connector_(host, serv),
      sfd_(null),
      endpoint_(null),
      phase_(CLOSED)
{
    pair<smartfd, bool> xy(tryconnect(connector_, endpoint_));
    sfd_ = xy.first;
    phase_ = xy.second ? ESTABLISHED : CONNECTING;
}
