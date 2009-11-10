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

#include "augaspp/cluster.hpp"

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
        static unsigned i((aug_irand() % 6) * 3);
        const unsigned j(i++);
        return static_cast<uint32_t>(base + j + ORDERING[j % 18]);
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

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

    class packet : public mpool_ops {
        char node_[AUG_PKTNODELEN + 1];
        unsigned sess_;
        aug_seqno_t seqno_;
    public:
        explicit
        packet(const char* node)
        {
            aug_strlcpy(node_, node, sizeof(node_));
            sess_ = getpid();
            seqno_ = 0;
        }
        size_t
        sendhbeat(sdref ref, const endpoint& ep)
        {
            struct aug_packet pkt;
            seqno_ = nextseq(100);
            aug_setpacket(node_, sess_, AUG_PKTHBEAT, seqno_, 0, 0, &pkt);

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(&pkt, buf);
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
        aug_seqno_t
        seqno() const
        {
            return seqno_;
        }
    };

    class receiver : public mpool_ops {
        sdref ref_;
        const endpoint& ep_;
        cluster cluster_;
        timer rdtimer_;
        timer wrtimer_;
        gaussian gauss_;
        packet out_;

        void
        flush()
        {
            aug_packet pkt;
            while (cluster_.next(pkt)) {
                switch (pkt.type_) {
                case AUG_PKTUP:
                    aug_ctxinfo(aug_tlx, "up message [%u]",
                                static_cast<unsigned>(pkt.seqno_));
                    break;
                case AUG_PKTDOWN:
                    aug_ctxinfo(aug_tlx, "down message [%u]",
                                static_cast<unsigned>(pkt.seqno_));
                    break;
                case AUG_PKTGAP:
                    aug_ctxinfo(aug_tlx, "gap message [%u]",
                                static_cast<unsigned>(pkt.seqno_));
                    break;
                default:
                    aug_ctxinfo(aug_tlx, "user message [%u]",
                                static_cast<unsigned>(pkt.seqno_));
                    break;
                }
            }
            stringstream ss;
            cluster_.print(ss);
            aug_ctxinfo(aug_tlx, "%s", ss.str().c_str());
        }
        void
        recv()
        {
            char buf[AUG_PACKETSIZE];
            endpoint from(null);
            if (AUG_PACKETSIZE != recvfrom(ref_, buf, sizeof(buf), 0, from))
                throw aug_error(__FILE__, __LINE__, AUG_EIO,
                                AUG_MSG("bad packet size"));
            aug_packet pkt;
            verify(aug_decodepacket(buf, &pkt));
            cluster_.insert(pkt);
        }

    public:
        ~receiver() AUG_NOTHROW
        {
        }
        receiver(sdref ref, const endpoint& ep, timers& ts)
            : ref_(ref),
              ep_(ep),
              cluster_(getclock(aug_tlx), 8, 2000),
              rdtimer_(ts, null),
              wrtimer_(ts, null),
              out_("test")
        {
            rdtimer_.set(cluster_.expiry(), *this);
            wrtimer_.set(1000, *this);
        }
        void
        timercb(aug_id id, unsigned& ms)
        {
            if (idref(id) == wrtimer_.id()) {

                out_.sendhbeat(ref_, ep_);
                aug_ctxinfo(aug_tlx, "send message [%u]",
                            static_cast<unsigned>(out_.seqno()));
                const double d(gauss_() * 400.0 + 600.0);
                ms = static_cast<unsigned>(AUG_MAX(d, 1.0));
                aug_ctxinfo(aug_tlx, "next send in %u ms", ms);

            } else if (idref(id) == rdtimer_.id()) {

                aug_ctxinfo(aug_tlx, "process timer");
                flush();
                ms = cluster_.expiry();
            }
        }
        void
        process()
        {
            try {
                for (;;)
                    recv();
            } catch (const block_exception&) {
            }
            flush();
            rdtimer_.set(cluster_.expiry(), *this);
        }
    };

    autosd
    mcastsd(const char* addr, unsigned short port, const char* ifname,
            endpoint& ep)
    {
        inetaddr in(addr);

        autosd sfd(aug::socket(family(in), SOCK_DGRAM));
        setnonblock(sfd, true);
        setreuseaddr(sfd, true);

        // Set outgoing multicast interface.

        if (ifname)
            setmcastif(sfd, ifname);

        const endpoint any(inetany(family(in)), htons(port));
        aug::bind(sfd, any);

        joinmcast(sfd, in, ifname);

        setfamily(ep, family(in));
        setport(ep, htons(port));
        setinetaddr(ep, in);
        return sfd;
    }

    void
    run(sdref ref, const endpoint& ep)
    {
        endpoint addr(null);
        getsockname(ref, addr);
        aug_ctxinfo(aug_tlx, "bound to [%s]", endpointntop(addr).c_str());

        muxer mux(getmpool(aug_tlx));
        timers ts(getmpool(aug_tlx), getclock(aug_tlx));

        receiver mrecv(ref, ep, ts);
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
                if (ready)
                    mrecv.process();
            } catch (const exception& e) {
                aug_ctxerror(aug_tlx, "%s", e.what());
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
                         "usage: packet <mcast> <port> [ifname]");
            return 1;
        }

        endpoint ep(null);
        autosd sfd(mcastsd(argv[1],
                           static_cast<unsigned short>(atoi(argv[2])),
                           4 == argc ? argv[3] : 0, ep));
        run(sfd, ep);
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
