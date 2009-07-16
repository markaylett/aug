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
#include "augservpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <csignal>
#include <iostream>

using namespace aug;
using namespace std;

// packet 239.1.1.1 17172 3

namespace {

    struct window_exception : std::exception { };

    struct discard_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::discard_exception";
        }
    };

    struct backlog_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::backlog_exception";
        }
    };

    struct timeout_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::timeout_exception";
        }
    };

    typedef unsigned long seqno_t;

    class window {
        enum state { SEED, SYNC, READY };
        struct message {
            aug_packet pkt_;
            aug_timeval tv_;
        };
        const unsigned size_;
        message* const ring_;
        seqno_t begin_, end_;
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
        explicit
        window(unsigned size)
            : size_(size),
              ring_(new message[size]),
              begin_(1),
              end_(1),
              state_(SEED)
        {
        }
        void
        insert(const aug_packet& pkt, const aug_timeval& tv)
        {
            aug_ctxinfo(aug_tlx, "insert: seqno=[%u]",
                        static_cast<unsigned>(pkt.seqno_));
            const seqno_t seqno(static_cast<seqno_t>(pkt.seqno_));
            if (SEED == state_) {
                const seqno_t inset((size_ - 1) / 2);
                if (seqno <= inset)
                    begin_ = seqno;
                else
                    begin_ = seqno - inset;
                aug_ctxinfo(aug_tlx, "seed: inset=[%u], begin=[%u]",
                            static_cast<unsigned>(inset),
                            static_cast<unsigned>(begin_));
                end_ = begin_;
                state_ = SYNC;
            } else {
                const long diff(seqno - begin_);
                if (diff < 0)
                    throw discard_exception();
                if (size_ <= static_cast<seqno_t>(diff)) {
                    if (SYNC == state_) {
                        aug_ctxcrit(aug_tlx,
                                    "backlog: diff=[%u], begin=[%u]",
                                    static_cast<unsigned>(diff),
                                    static_cast<unsigned>(begin_));
                        do {
                            aug_ctxinfo(aug_tlx, "freeing");
                            ++begin_;
                        } while (size_ <= seqno - begin_);
                    } else
                        throw backlog_exception();
                }
            }
            if (end_ <= seqno) {
                for (seqno_t i(end_); i < seqno; ++i) {
                    aug_ctxcrit(aug_tlx, "clear: i=[%u]",
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
            state_ = READY;
            message& m(ring_[begin_++ % size_]);
            memcpy(&pkt, &m.pkt_, sizeof(pkt));
            tv.tv_sec = m.tv_.tv_sec;
            tv.tv_usec = m.tv_.tv_usec;
            clear(m);
            return true;
        }
        void
        drop()
        {
            if (begin_ != end_ && empty(ring_[begin_ % size_]))
                ++begin_;
        }
        void
        trim()
        {
            while (begin_ != end_ && empty(ring_[begin_ % size_]))
                ++begin_;
            aug_ctxinfo(aug_tlx, "trim: begin=[%u]",
                        static_cast<unsigned>(begin_));
        }
        bool
        empty() const
        {
            return begin_ == end_ || empty(ring_[begin_ % size_]);
        }
        bool
        ready() const
        {
            return READY == state_;
        }
    };

    class expirywindow {
        clockptr clock_;
        window window_;
        aug_timeval timeout_;
        aug_timeval expiry_;
    public:
        explicit
        expirywindow(clockref clock, unsigned size, unsigned timeout)
            : clock_(object_retain(clock)),
              window_(size)
        {
            // 20% tolerance.
            mstotv(timeout + timeout / 5, timeout_);

            gettimeofday(clock, expiry_);
            tvadd(expiry_, timeout_);
        }
        void
        insert(const aug_packet& pkt)
        {
            aug_timeval tv;
            gettimeofday(clock_, tv);
            window_.insert(pkt, tv);
        }
        bool
        next(aug_packet& pkt)
        {
            aug_timeval tv;
            if (window_.empty()) {
                gettimeofday(clock_, tv);
                if (timercmp(&expiry_, &tv, <=)) {
                    // Advance expiry time.
                    tvadd(expiry_, timeout_);
                    if (window_.ready()) {
                        // Expiry time has passed.
                        window_.drop();
                        throw timeout_exception();
                    } else {
                        window_.trim();
                        if (window_.empty()) {
                            // Not seeded.
                            return false;
                        }
                    }
                } else
                    return false;
            }
            window_.next(pkt, tv);
            tvadd(tv, timeout_);
            if (timercmp(&expiry_, &tv, <)) {
                // Advance expiry time.
                expiry_.tv_sec = tv.tv_sec;
                expiry_.tv_usec = tv.tv_usec;
            }
            return true;
        }
        unsigned
        expiry() const
        {
            // Milliseconds to expiry.
            aug_timeval tv, now;
            tv.tv_sec = expiry_.tv_sec;
            tv.tv_usec = expiry_.tv_usec;
            gettimeofday(clock_, now);
            const unsigned ms(tvtoms(tvsub(tv, now)));
            aug_ctxinfo(aug_tlx, "expiry: tv_sec=[%d], ms=[%u]",
                        static_cast<int>(tv.tv_sec),
                        static_cast<unsigned>(ms));
            if (0 == ms)
                exit(0);
            return ms;
        }
    };

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

    class packet : aug_packet, public mpool_ops {
        size_t
        sendhead(sdref ref, unsigned type, const endpoint& ep)
        {
            ++seqno_;
            verno_ = 1;
            time_ = static_cast<uint64_t>(time(0) * 1000);
            flags_ = 0;
            type_ = static_cast<uint16_t>(type);

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(static_cast<aug_packet*>(this), buf);
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
    public:
        explicit
        packet(const char* chan)
        {
            proto_ = 1;
            aug_strlcpy(chan_, chan, sizeof(chan_));
            seqno_ = 100;
            verno_ = 0;
            time_ = 0;
            flags_ = 0;
            type_ = 0;
        }
        size_t
        sendhbeat(sdref ref, const endpoint& ep)
        {
            return sendhead(ref, 1, ep);
        }
        size_t
        sendevent(sdref ref, const char* data,
                  const endpoint& ep)
        {
            aug_strlcpy(data_, data, sizeof(data_));
            return sendhead(ref, 2, ep);
        }
    };

    class session : public mpool_ops {
        sdref ref_;
        const endpoint& ep_;
        expirywindow window_;
        timer rdwait_;
        timer wrwait_;
        packet out_;

    public:
        ~session() AUG_NOTHROW
        {
        }
        session(sdref ref, const endpoint& ep, timers& ts)
            : ref_(ref),
              ep_(ep),
              window_(getclock(aug_tlx), 8, 3000),
              rdwait_(ts, null),
              wrwait_(ts, null),
              out_("test")
        {
            rdwait_.set(window_.expiry(), *this);
            wrwait_.set(200, *this);
        }
        void
        recv(sdref ref)
        {
            char buf[AUG_PACKETSIZE];
            endpoint from(null);
            recvfrom(ref, buf, sizeof(buf), 0, from);
            aug_packet pkt;
            verify(aug_decodepacket(buf, &pkt));
            window_.insert(pkt);
        }
        void
        timercb(aug_id id, unsigned& ms)
        {
            if (idref(id) == wrwait_.id()) {

                aug_ctxinfo(aug_tlx, "wrwait timeout");
                out_.sendhbeat(ref_, ep_);
            } else if (idref(id) == rdwait_.id()) {

                aug_ctxinfo(aug_tlx, "rdwait timeout");
                process();
            }
        }
        void
        process()
        {
            try {
                aug_packet pkt;
                while (window_.next(pkt)) {
                  aug_ctxinfo(aug_tlx, "next: seqno=[%u], type=[%u]",
                              static_cast<unsigned>(pkt.seqno_),
                              static_cast<unsigned>(pkt.type_));
                }
            } catch (const window_exception& e) {
                aug_ctxerror(aug_tlx, "error: %s", e.what());
            }
            rdwait_.set(window_.expiry(), *this);
        }
    };

    void
    run(sdref ref, const endpoint& ep)
    {
        endpoint addr(null);
        getsockname(ref, addr);
        aug_ctxinfo(aug_tlx, "bound to: [%s]", endpointntop(addr).c_str());

        muxer mux(getmpool(aug_tlx));
        timers ts(getmpool(aug_tlx), getclock(aug_tlx));
        session sess(ref, ep, ts);
        setmdeventmask(mux, ref, AUG_MDEVENTRDEX);

        aug_timeval tv;
        unsigned ready(!0);
        while (!stop_) {

            processexpired(ts, 0 == ready, tv);
            aug_ctxinfo(aug_tlx, "hbeat in: tv_sec=%d, tv_usec=%d",
                        static_cast<int>(tv.tv_sec),
                        static_cast<int>(tv.tv_usec));

            try {
                ready = waitmdevents(mux, tv);
            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            aug_ctxinfo(aug_tlx, "waitmdevents: %u", ready);

            if (ready)
                sess.recv(ref);

            if (0 == aug_rand() % 7)
                sess.process();
        }
    }
}

int
main(int argc, char* argv[])
{
    signal(SIGINT, sigcatch);

    try {

        autotlx();
        setdaemonlog(aug_tlx);

        aug_timeval tv;
        gettimeofday(getclock(aug_tlx), tv);
        aug::srand(getpid() ^ tv.tv_sec ^ tv.tv_usec);

        if (argc < 3) {
            aug_ctxerror(aug_tlx,
                         "usage: packet <mcast> <serv> [ifname]");
            return 1;
        }

        inetaddr in(argv[1]);
        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        setreuseaddr(sfd, true);

        // Set outgoing multicast interface.

        if (4 == argc)
            setmcastif(sfd, argv[3]);

        // Don't receive packets from self.

        endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
        aug::bind(sfd, ep);

        joinmcast(sfd, in, 4 == argc ? argv[3] : 0);
        setinetaddr(ep, in);
        run(sfd, ep);
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
