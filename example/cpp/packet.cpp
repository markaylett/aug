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

#include <cmath>
#include <csignal>
#include <iostream>

using namespace aug;
using namespace std;

// packet 239.1.1.1 17172 3

namespace {

    const int ORDERING[] = {
    //  1   2   3
    //  1   2   3
        0,  0,  0,
    //  1   3   2
    //  1   2   3
        0,  1, -1,
    //  2   1   3
    //  1   2   3
        1, -1,  0,
    //  2   3   1
    //  1   2   3
        1,  1, -2,
    //  3   1   2
    //  1   2   3
        2, -1, -1,
    //  3   2   1
    //  1   2   3
        2,  0, -2
    };

    uint32_t
    nextseq(unsigned base)
    {
        static unsigned i(1 + (aug_irand() % 6) * 3);
        const unsigned j(i++);
        return static_cast<uint32_t>(base + j + ORDERING[j % 18]);
    }

    bool
    tvbefore(const aug_timeval& lhs, const aug_timeval& rhs)
    {
        aug_timeval local = { 0, 5000 };
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

    class gaussian {
        double y2_;
        bool empty_;
    public:
        gaussian()
            : empty_(true)
        {
        }
        double
        operator ()()
        {
            double y1;
            if (empty_) {
                double x1, x2, w;
                do {
                    x1 = 2.0 * drand() - 1.0;
                    x2 = 2.0 * drand() - 1.0;
                    w = x1 * x1 + x2 * x2;
                } while (1.0 <= w);
                w = sqrt((-2.0 * std::log(w)) / w);
                y1 = x1 * w;
                y2_ = x2 * w;
                empty_ = false;
            } else {
                y1 = y2_;
                empty_ = true;
            }
            return y1;
        }
    };

    struct window_exception : std::exception { };

    struct discard_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::discard_exception";
        }
    };

    struct overflow_exception : window_exception {
        const char*
        what() const throw()
        {
            return "aug::overflow_exception";
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
            aug_ctxinfo(aug_tlx, "inserting message [%u]",
                        static_cast<unsigned>(pkt.seqno_));
            const seqno_t seqno(static_cast<seqno_t>(pkt.seqno_));
            if (SEED == state_) {
                const seqno_t inset((size_ - 1) / 2);
                if (seqno <= inset)
                    begin_ = seqno;
                else
                    begin_ = seqno - inset;
                aug_ctxinfo(aug_tlx, "seed at offset [%u] from [%u]",
                            static_cast<unsigned>(inset),
                            static_cast<unsigned>(begin_));
                end_ = begin_;
                state_ = SYNC;
            } else {
                long diff(seqno - begin_);
                if (diff < 0)
                    throw discard_exception();
                diff -= size_;
                if (0 < diff) {
                    if (SYNC == state_) {
                        aug_ctxcrit(aug_tlx,
                                    "overflow by [%u] from [%u]",
                                    static_cast<unsigned>(diff),
                                    static_cast<unsigned>(begin_));
                        begin_ += static_cast<seqno_t>(diff);
                    } else
                        throw overflow_exception();
                }
            }
            if (end_ <= seqno) {
                for (seqno_t i(end_); i < seqno; ++i) {
                    aug_ctxinfo(aug_tlx, "clear intermediate [%u]",
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
            //state_ = READY;
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
            aug_ctxinfo(aug_tlx, "trimmed to [%u]",
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
            aug_ctxinfo(aug_tlx, "initial expiry [%s]",
                        tvstring(expiry_).c_str());
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
                if (tvbefore(tv, expiry_)) {
                    // Advance expiry time.
                    tvadd(expiry_, timeout_);
                    aug_ctxinfo(aug_tlx, "moved expiry [%s]",
                                tvstring(expiry_).c_str());
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
                aug_ctxinfo(aug_tlx, "set expiry [%s]",
                            tvstring(expiry_).c_str());
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
            return tvtoms(tvsub(tv, now));
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
            seqno_ = nextseq(100);
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
            seqno_ = 0;
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
        seqno_t
        seqno() const
        {
            return seqno_;
        }
    };

    class session : public mpool_ops {
        sdref ref_;
        const endpoint& ep_;
        expirywindow window_;
        timer rdwait_;
        timer wrwait_;
        gaussian gauss_;
        packet out_;

    public:
        ~session() AUG_NOTHROW
        {
        }
        session(sdref ref, const endpoint& ep, timers& ts)
            : ref_(ref),
              ep_(ep),
              window_(getclock(aug_tlx), 8, 20000),
              rdwait_(ts, null),
              wrwait_(ts, null),
              out_("test")
        {
            rdwait_.set(window_.expiry(), *this);
            wrwait_.set(1000, *this);
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

                out_.sendhbeat(ref_, ep_);
                aug_ctxinfo(aug_tlx, "send message [%u]",
                            static_cast<unsigned>(out_.seqno()));
                const double d(gauss_() * 400.0 + 600.0);
                ms = static_cast<unsigned>(AUG_MAX(d, 1.0));
                aug_ctxinfo(aug_tlx, "next message in %u ms", ms);

            } else if (idref(id) == rdwait_.id()) {

                aug_ctxinfo(aug_tlx, "process timer");
                try {
                    process();
                } catch (const window_exception& e) {
                    aug_ctxerror(aug_tlx, "error: %s", e.what());
                }
                ms = window_.expiry();
            }
        }
        void
        process()
        {
            aug_packet pkt;
            while (window_.next(pkt)) {
                aug_ctxinfo(aug_tlx, "recv message [%u]",
                            static_cast<unsigned>(pkt.seqno_));
            }
        }
        void
        setexpiry()
        {
            rdwait_.set(window_.expiry(), *this);
        }
    };

    void
    run(sdref ref, const endpoint& ep)
    {
        endpoint addr(null);
        getsockname(ref, addr);
        aug_ctxinfo(aug_tlx, "bound to [%s]", endpointntop(addr).c_str());

        muxer mux(getmpool(aug_tlx));
        timers ts(getmpool(aug_tlx), getclock(aug_tlx));
        session sess(ref, ep, ts);
        setmdeventmask(mux, ref, AUG_MDEVENTRDEX);

        aug_timeval tv;
        unsigned ready(!0);
        while (!stop_) {

            processexpired(ts, 0 == ready, tv);

            try {
                ready = waitmdevents(mux, tv);
            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            try {
                if (ready) {
                    sess.recv(ref);
                    sess.process();
                    sess.setexpiry();
                }
            } catch (const discard_exception& e) {
                aug_ctxerror(aug_tlx, "%s", e.what());
            } catch (const overflow_exception& e) {
                aug_ctxerror(aug_tlx, "%s", e.what());
                break;
            }
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
        setloglevel(aug_tlx, 99);

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
