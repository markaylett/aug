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
#include "augaspp/cluster.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsyspp/smartptr.hpp"
#include "augsyspp/time.hpp"

#include "augctxpp/exception.hpp"

#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>

#if defined(min)
# undef min
#endif // min

#if defined(max)
# undef max
#endif // max

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

    struct window_exception : std::exception {
        ~window_exception() AUG_NOTHROW
        {
        }
    };

    struct overflow_exception : window_exception {
        ~overflow_exception() AUG_NOTHROW
        {
        }
        const char*
        what() const throw()
        {
            return "aug::overflow_exception";
        }
    };

    struct timeout_exception : window_exception {
        ~timeout_exception() AUG_NOTHROW
        {
        }
        const char*
        what() const throw()
        {
            return "aug::timeout_exception";
        }
    };

    struct underflow_exception : window_exception {
        ~underflow_exception() AUG_NOTHROW
        {
        }
        const char*
        what() const throw()
        {
            return "aug::underflow_exception";
        }
    };

    struct message {
        aug_packet pkt_;
        aug_timeval tv_;
    };

    enum state {
        // First packet pending.
        RESET,
        // Initial packet ordering.
        SYNC,
        // Fully initialised state.
        READY
    };

    class window {
        const unsigned size_;
        message* const ring_;
        aug_seqno_t begin_, end_;
        state state_;

        static void
        clear(message& m)
        {
            m.tv_.tv_sec = 0;
        }

        static bool
        empty(const message& m)
        {
            return 0 == m.tv_.tv_sec;
        }

    public:
        ~window() AUG_NOTHROW
        {
            delete[] ring_;
        }

        window(unsigned size)
            : size_(size),
              ring_(new message[size]),
              begin_(1),
              end_(1),
              state_(RESET)
        {
        }

        void
        drop()
        {
            // Drop the first packet from the left of the window.

            if (begin_ != end_)
                clear(ring_[begin_++ % size_]);
        }

        void
        flush()
        {
            // Flush any space to the left of the window.

            while (begin_ != end_ && empty(ring_[begin_ % size_]))
                ++begin_;
            aug_ctxinfo(aug_tlx, "flush to [%u]",
                        static_cast<unsigned>(begin_));
            state_ = READY;
        }

        void
        insert(const aug_packet& pkt, const aug_timeval& tv)
        {
            AUG_CTXDEBUG2(aug_tlx, "insert message [%u]",
                          static_cast<unsigned>(pkt.seqno_));

            const aug_seqno_t seqno(static_cast<aug_seqno_t>(pkt.seqno_));

            if (RESET == state_) {

                // To seed the first packet, choose an initial insertion point
                // half way through the window.  This allows some space before
                // the first item to deal with out of order conditions.  This
                // can occur, for example, where the first packet is received
                // out of order, so that the next packet actually has a lower
                // sequence number.

                const aug_seqno_t inset((size_ - 1) / 2);
                if (seqno <= inset) {

                    // Seed with the actual sequence number when the sequence
                    // number is less than the insertion point.  Sequence
                    // number one, for example, would always be inserted at
                    // position one.

                    begin_ = seqno;

                } else
                    begin_ = seqno - inset;

                AUG_CTXDEBUG2(aug_tlx, "seed from [%u] at offset [%u]",
                              static_cast<unsigned>(begin_),
                              static_cast<unsigned>(inset));

                end_ = begin_;
                state_ = SYNC;

            } else {

                // Discard any packets that fall to the left of the window.
                // In most cases, these can be dismissed as duplicates.

                long diff(seqno - begin_);
                if (diff < 0)
                    throw underflow_exception();;

                diff -= size_ - 1;

                if (0 < diff) {

                    // When fully initialised, discard any packets that exceed
                    // the maximum window size.

                    if (ready())
                        throw overflow_exception();

                    // Otherwise this is the SYNC state.  The SYNC state
                    // exists after the first packet has been received, but
                    // before flush() has been called.  While in this state,
                    // it is safe to discard packets from the left of the
                    // window to make space on the right, because none have
                    // been consumed.

                    aug_ctxwarn(aug_tlx, "overflow by [%u] from [%u]",
                                static_cast<unsigned>(diff),
                                static_cast<unsigned>(begin_));

                    // Drop sufficient packets to allow insert.

                    do {
                        AUG_CTXDEBUG2(aug_tlx, "clear lead [%u]",
                                      static_cast<unsigned>(begin_));
                        clear(ring_[begin_++ % size_]);
                    } while (0 < --diff);
                    begin_ += static_cast<aug_seqno_t>(diff);
                }
            }
            if (end_ <= seqno) {

                // Clear gaps between the previous and latest packet.

                for (aug_seqno_t i(end_); i < seqno; ++i) {
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

        bool
        next(aug_packet& pkt, aug_timeval& tv)
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

        void
        reset()
        {
            state_ = RESET;
        }

        void
        print(ostream& os) const
        {
            // To describe a window from 100 to 107 inclusive:
            //
            // 100---+++-+:SYNC
            //
            // Where '-' is a gap;
            // and '+' is a message.

            os << begin_;
            for (aug_seqno_t i(begin_); i < end_; ++i)
                os << (empty(ring_[i % size_]) ? '-' : '+');
            switch (state_) {
            case RESET:
                os << ":RESET";
                break;
            case SYNC:
                os << ":SYNC";
                break;
            case READY:
                os << ":READY";
                break;
            }
        }

        bool
        empty() const
        {
            return begin_ == end_
                || empty(ring_[begin_ % size_])
                || !ready();
        }

        bool
        ready() const
        {
            return READY == state_;
        }
    };

    class node {
        window window_;
        aug_timeval timeout_;
        aug_timeval expiry_;
    public:
        node(unsigned size, const aug_timeval& now,
             unsigned timeout)
            : window_(size)
        {
            // 20% tolerance.

            mstotv(timeout + timeout / 5, timeout_);

            expiry_.tv_sec = now.tv_sec;
            expiry_.tv_usec = now.tv_usec;
            tvadd(expiry_, timeout_);

            AUG_CTXDEBUG2(aug_tlx, "initial expiry [%s]",
                          tvstring(expiry_).c_str());
        }

        void
        insert(const aug_packet& pkt, const aug_timeval& now)
        {
            window_.insert(pkt, now);
        }

        bool
        next(aug_packet& pkt, const aug_timeval& now)
        {
            aug_timeval tv = { now.tv_sec, now.tv_usec };

            if (window_.empty()) {

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

                        // Move to a ready state.

                        window_.flush();
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
                AUG_CTXDEBUG2(aug_tlx, "set expiry [%s]",
                              tvstring(expiry_).c_str());
            }
            return true;
        }

        void
        reset(const aug_timeval& now)
        {
            expiry_.tv_sec = now.tv_sec;
            expiry_.tv_usec = now.tv_usec;
            tvadd(expiry_, timeout_);

            window_.reset();
        }

        void
        print(ostream& os) const
        {
            window_.print(os);
            os << ' '  << tvstring(expiry_);
        }

        unsigned
        expiry(const aug_timeval& now) const
        {
            // Milliseconds to expiry.

            aug_timeval tv = { expiry_.tv_sec, expiry_.tv_usec };
            return tvtoms(tvsub(tv, now));
        }
    };

    typedef smartptr<node> nodeptr;
}

namespace aug {

    namespace detail {

        struct clusterimpl : mpool_ops {

            clockptr clock_;
            const unsigned size_;
            const unsigned timeout_;
            map<pair<unsigned, string>, nodeptr> nodes_;
            queue<aug_packet> pending_;

            clusterimpl(clockref clock, unsigned size,
                        unsigned timeout)
                : clock_(object_retain(clock)),
                  size_(size),
                  timeout_(timeout)
            {
            }

            void
            flush()
            {
                aug_timeval now;
                gettimeofday(clock_, now);

                map<pair<unsigned, string>, nodeptr>::iterator
                    it(nodes_.begin()), end(nodes_.end());
                while (it != end) {
                    aug_packet out;
                    try {
                        while (it->second->next(out, now))
                            pending_.push(out);
                    } catch (const timeout_exception& e) {

                        aug_ctxwarn(aug_tlx, "timeout: %s", e.what());

                        // Synthetic fin packet.

                        aug_setpacket(it->first.second.c_str(),
                                      it->first.first, 0, AUG_PKTFIN, 0, 0,
                                      &out);
                        pending_.push(out);

                        nodes_.erase(it++);
                    }
                }
            }
        };
    }
}

AUGASPP_API
cluster::~cluster() AUG_NOTHROW
{
    delete impl_;
}

AUGASPP_API
cluster::cluster(clockref clock, unsigned wsize, unsigned timeout)
    : impl_(new (tlx) detail::clusterimpl(clock, wsize, timeout))
{
}

AUGASPP_API bool
cluster::insert(const aug_packet& pkt)
{
    aug_timeval now;
    gettimeofday(impl_->clock_, now);
    aug_packet out;

    const pair<unsigned, string> key
        (pkt.sess_, string(pkt.node_, sizeof(pkt.node_)));
    map<pair<unsigned, string>, nodeptr>
        ::iterator it(impl_->nodes_.find(key));
    if (it == impl_->nodes_.end()) {

        // Create new node.

        it = impl_->nodes_.insert
            (make_pair(key, nodeptr
                       (new node(impl_->size_, now, impl_->timeout_)))).first;

        aug_setpacket(key.second.c_str(), key.first,
                      0, AUG_PKTFIN, 0, 0, &out);
        impl_->pending_.push(out);
    }

    try {
        it->second->insert(pkt, now);
    } catch (const overflow_exception& e) {
        aug_ctxwarn(aug_tlx, "resetting: %s", e.what());
        it->second->reset(now);
        it->second->insert(pkt, now);
    } catch (const underflow_exception&) {
        return false;
    }

    try {
        while (it->second->next(out, now))
            impl_->pending_.push(out);
    } catch (const timeout_exception& e) {

        aug_ctxwarn(aug_tlx, "timeout: %s", e.what());

        // Synthetic fin packet.

        aug_setpacket(key.second.c_str(), key.first,
                      0, AUG_PKTFIN, 0, 0, &out);
        impl_->pending_.push(out);

        impl_->nodes_.erase(it);
    }

    return true;
}

AUGASPP_API bool
cluster::next(aug_packet& pkt)
{
    if (impl_->pending_.empty()) {
        impl_->flush();
        if (impl_->pending_.empty())
            return false;
    }
    memcpy(&pkt, &impl_->pending_.front(), sizeof(pkt));
    impl_->pending_.pop();
    return true;
}

AUGASPP_API void
cluster::print(ostream& os) const
{
    map<pair<unsigned, string>, nodeptr>::const_iterator
        it(impl_->nodes_.begin()), end(impl_->nodes_.end());
    for (; it != end; ++it) {
        it->second->print(os);
        os << endl;
    }
}

AUGASPP_API unsigned
cluster::expiry() const
{
    aug_timeval now;
    gettimeofday(impl_->clock_, now);

    unsigned min(numeric_limits<unsigned>::max());
    map<pair<unsigned, string>, nodeptr>::const_iterator
        it(impl_->nodes_.begin()), end(impl_->nodes_.end());
    for (; it != end; ++it) {
        const unsigned ms(it->second->expiry(now));
        if (ms < min)
            min = ms;
    }
    return min;
}
