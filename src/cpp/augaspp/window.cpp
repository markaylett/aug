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
#include "augaspp/window.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsyspp/time.hpp"

#include "augctxpp/exception.hpp"

#include <iostream>
#include <sstream>

using namespace aug;
using namespace std;

namespace {

    const aug_suseconds EPSILONUS(5000);

    bool
    tvbefore(const aug_timeval& lhs, const aug_timeval& rhs)
    {
        aug_timeval local = { 0, EPSILONUS };
        tvadd(local, lhs);
        return timercmp(&rhs, &local, <) ? true : false;
    }

    string
    tvstring(const aug_timeval& tv)
    {
        struct tm local;
        aug_localtime(&tv.tv_sec, &local);

        char buf[256];
        strftime(buf, sizeof(buf), "%H:%M:%S", &local);

        stringstream ss;
        ss << buf << '.' << (tv.tv_usec / 1000);
        return ss.str();
    }
}

AUGASPP_API
window_exception::~window_exception() AUG_NOTHROW
{
}

AUGASPP_API
discard_exception::~discard_exception() AUG_NOTHROW
{
}

AUGASPP_API const char*
discard_exception::what() const throw()
{
    return "aug::discard_exception";
}

AUGASPP_API
timeout_exception::~timeout_exception() AUG_NOTHROW
{
}

AUGASPP_API const char*
timeout_exception::what() const throw()
{
    return "aug::timeout_exception";
}

AUGASPP_API void
window::clear(message& m)
{
    m.tv_.tv_sec = 0;
}

AUGASPP_API bool
window::empty(const message& m)
{
    return 0 == m.tv_.tv_sec;
}

AUGASPP_API
window::~window() AUG_NOTHROW
{
    delete[] ring_;
}

AUGASPP_API
window::window(unsigned size)
    : size_(size),
      ring_(new message[size]),
      begin_(1),
      end_(1),
      state_(START)
{
}

AUGASPP_API void
window::insert(const aug_packet& pkt, const aug_timeval& tv)
{
    AUG_CTXDEBUG2(aug_tlx, "insert message [%u]",
                  static_cast<unsigned>(pkt.seqno_));

    const seqno_t seqno(static_cast<seqno_t>(pkt.seqno_));

    if (START == state_) {

        // To seed the first packet, choose an initial insertion point half
        // way through the window.  This allows some space before the first
        // item to deal with out of order conditions.  This can occur, for
        // example, where the first packet is received out of order, so that
        // the next packet actually has a lower sequence number.

        const seqno_t inset((size_ - 1) / 2);
        if (inset < seqno) {
            begin_ = seqno - inset;
        } else {

            // Seed with the actual sequence number if it is less than the
            // insertion point.

            begin_ = seqno;
        }

        AUG_CTXDEBUG2(aug_tlx, "seed from [%u] at offset [%u]",
                      static_cast<unsigned>(begin_),
                      static_cast<unsigned>(inset));

        end_ = begin_;
        state_ = PRIME;

    } else {

        // Discard any packets that fall to the left of the window.  In most
        // cases, these can be dismissed as duplicates.

        long diff(seqno - begin_);
        if (diff < 0)
            throw discard_exception();
        diff -= size_ - 1;

        if (0 < diff) {

            // When fully initialised, discard any packets that exceed the
            // maximum window size.

            if (ready())
                throw aug_error(__FILE__, __LINE__, AUG_ERANGE,
                                AUG_MSG("maximum window size"));

            // Otherwise this is the PRIME state.  The PRIME state exists
            // after the first packet has been received, but before sync() has
            // been called.  While in this state, it is safe to discard
            // packets from the left of the window to make space on the right,
            // because none have been consumed.

            aug_ctxwarn(aug_tlx, "overflow by [%u] from [%u]",
                        static_cast<unsigned>(diff),
                        static_cast<unsigned>(begin_));

            // Drop sufficient packets to allow insert.

            do {
                AUG_CTXDEBUG2(aug_tlx, "clear lead [%u]",
                            static_cast<unsigned>(begin_));
                clear(ring_[begin_++ % size_]);
            } while (0 < --diff);
            begin_ += static_cast<seqno_t>(diff);
        }
    }
    if (end_ <= seqno) {

        // Clear gaps between the previous and latest packet.

        for (seqno_t i(end_); i < seqno; ++i) {
            AUG_CTXDEBUG2(aug_tlx, "clear gap [%u]",
                          static_cast<unsigned>(i));
            clear(ring_[i % size_]);
        }
        end_ = seqno + 1;
    }
    message& m(ring_[seqno % size_]);
    memcpy(&m.pkt_, &pkt, sizeof(m.pkt_));
    m.tv_.tv_sec = tv.tv_sec;
    m.tv_.tv_usec = tv.tv_usec;
}

AUGASPP_API bool
window::next(aug_packet& pkt, aug_timeval& tv)
{
    if (empty())
        return false;

    // Next message to output arguments.

    message& m(ring_[begin_++ % size_]);
    memcpy(&pkt, &m.pkt_, sizeof(pkt));
    tv.tv_sec = m.tv_.tv_sec;
    tv.tv_usec = m.tv_.tv_usec;

    // Clear consumed.

    clear(m);
    return true;
}

AUGASPP_API void
window::drop()
{
    // Drop the first packet from the left of the window.

    if (begin_ != end_)
        clear(ring_[begin_++ % size_]);
}

AUGASPP_API void
window::print(ostream& os) const
{
    // To describe a window from 100 to 107 inclusive:
    //
    // 100---+++-+:SYNC
    //
    // Where '-' is a gap;
    // and '+' is a message.

    os << begin_;
    for (seqno_t i(begin_); i < end_; ++i)
        os << (empty(ring_[i % size_]) ? '-' : '+');
    switch (state_) {
    case START:
        os << ":START";
        break;
    case PRIME:
        os << ":PRIME";
        break;
    case READY:
        os << ":READY";
        break;
    }
}

AUGASPP_API void
window::sync()
{
    // Flush any space to the left of the window.

    while (begin_ != end_ && empty(ring_[begin_ % size_]))
        ++begin_;
    aug_ctxinfo(aug_tlx, "sync to [%u]", static_cast<unsigned>(begin_));
    state_ = READY;
}

AUGASPP_API bool
window::empty() const
{
    return begin_ == end_
        || empty(ring_[begin_ % size_])
        || !ready();
}

AUGASPP_API bool
window::ready() const
{
    return READY == state_;
}

AUGASPP_API
expirywindow::expirywindow(clockref clock, unsigned size, unsigned timeout)
    : clock_(object_retain(clock)),
      window_(size)
{
    // 20% tolerance.

    mstotv(timeout + timeout / 5, timeout_);

    gettimeofday(clock, expiry_);
    tvadd(expiry_, timeout_);
    AUG_CTXDEBUG2(aug_tlx, "initial expiry [%s]", tvstring(expiry_).c_str());
}

AUGASPP_API void
expirywindow::insert(const aug_packet& pkt)
{
    aug_timeval tv;
    gettimeofday(clock_, tv);
    window_.insert(pkt, tv);
}

AUGASPP_API bool
expirywindow::next(aug_packet& pkt)
{
    aug_timeval tv;

    if (window_.empty()) {

        gettimeofday(clock_, tv);
        if (tvbefore(tv, expiry_)) {

            // Advance expiry time.

            tvadd(expiry_, timeout_);
            AUG_CTXDEBUG2(aug_tlx, "moved expiry [%s]",
                        tvstring(expiry_).c_str());

            if (window_.ready()) {

                // Drop timed-out packet from the window.

                window_.drop();
                throw timeout_exception();

            } else {

                // Move a ready state.

                window_.sync();
                if (window_.empty()) {

                    // No packet received.
                    return false;
                }
            }
        } else {

            // Timer has not expired.
            return false;
        }
    }

    // Get the next packet.

    window_.next(pkt, tv);

    // Add the timeout value to the time it was received.

    tvadd(tv, timeout_);

    // Update the timer if this time is before the current expiry
    // time.

    if (timercmp(&expiry_, &tv, <)) {

        // Advance expiry time.

        expiry_.tv_sec = tv.tv_sec;
        expiry_.tv_usec = tv.tv_usec;
        AUG_CTXDEBUG2(aug_tlx, "set expiry [%s]", tvstring(expiry_).c_str());
    }
    return true;
}

AUGASPP_API void
expirywindow::print(ostream& os) const
{
    window_.print(os);
    os << ' '  << tvstring(expiry_);
}

AUGASPP_API unsigned
expirywindow::expiry() const
{
    // Milliseconds to expiry.

    aug_timeval tv, now;
    tv.tv_sec = expiry_.tv_sec;
    tv.tv_usec = expiry_.tv_usec;
    gettimeofday(clock_, now);
    return tvtoms(tvsub(tv, now));
}
