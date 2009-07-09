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

// packet 239.1.1.1 1000 3

namespace {

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

    void
    recvfrom(sdref ref, aug_packet& pkt, endpoint& ep)
    {
        char buf[AUG_PACKETSIZE];
        aug::recvfrom(ref, buf, sizeof(buf), 0, ep);
        aug_decodepacket(&pkt, buf);
    }

    class packet : aug_packet, public mpool_ops {
    public:
        explicit
        packet(const char* addr)
        {
            ver_ = 1;
            type_ = 0;
            seq_ = 0;
            aug_strlcpy(addr_, addr, sizeof(addr_));
        }
        size_t
        sendhbeat(sdref ref, const endpoint& ep)
        {
            type_ = AUG_PKTHBEAT;
            ++seq_;

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(buf, static_cast<aug_packet*>(this));
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
        size_t
        sendreset(sdref ref, unsigned next, const endpoint& ep)
        {
            type_ = AUG_PKTRESET;
            ++seq_;
            content_.reset_.next_ = next;

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(buf, static_cast<aug_packet*>(this));
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
        size_t
        sendevent(sdref ref, const char* method, const char* uri,
                  const endpoint& ep)
        {
            type_ = AUG_PKTEVENT;
            ++seq_;
            aug_strlcpy(content_.event_.method_, method,
                        sizeof(content_.event_.method_));
            aug_strlcpy(content_.event_.uri_, uri,
                        sizeof(content_.event_.uri_));

            char buf[AUG_PACKETSIZE];
            aug_encodepacket(buf, static_cast<aug_packet*>(this));
            return aug::sendto(ref, buf, sizeof(buf), 0, ep);
        }
    };

    class session : public mpool_ops {
        sdref ref_;
        const endpoint& ep_;
        timer hbwait_;

    public:
        ~session() AUG_NOTHROW
        {
        }
        session(sdref ref, const endpoint& ep, timers& ts)
            : ref_(ref),
              ep_(ep),
              hbwait_(ts, null)
        {
            hbwait_.set(2000, *this);
        }
        void
        recvd(const aug_packet& pkt, const endpoint& from)
        {
            aug_ctxinfo(aug_tlx, "recv: type=[%u], seq=[%u]", pkt.type_,
                        pkt.seq_);
        }
        void
        timercb(aug_id id, unsigned& ms)
        {
            if (idref(id) == hbwait_.id()) {

                aug_ctxinfo(aug_tlx, "hbint timeout");
                packet pkt("test");
                pkt.sendhbeat(ref_, ep_);
            }
        }
    };

    void
    run(sdref ref, const endpoint& ep)
    {
        endpoint addr(null);
        getsockname(ref, addr);
        aug_ctxinfo(aug_tlx, "bound to: [%s]", endpointntop(addr).c_str());

        muxer mux(getmpool(aug_tlx));
        timers ts(getmpool(aug_tlx));
        session sess(ref, ep, ts);
        setmdeventmask(mux, ref, AUG_MDEVENTRDEX);

        aug_timeval tv;
        unsigned ready(!0);
        while (!stop_) {

            processexpired(ts, 0 == ready, tv);
            aug_ctxinfo(aug_tlx, "timeout in: tv_sec=%d, tv_usec=%d",
                        (int)tv.tv_sec, (int)tv.tv_usec);

            try {
                ready = waitmdevents(mux, tv);
            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            aug_ctxinfo(aug_tlx, "waitmdevents: %u", ready);

            if (ready) {
                aug_packet pkt;
                endpoint from(null);
                recvfrom(ref, pkt, from);
                sess.recvd(pkt, from);
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
        aug_setlog(aug_tlx, aug_getdaemonlog());

        aug_timeval tv;
        aug::gettimeofday(tv);

        if (argc < 3) {
            aug_ctxerror(aug_tlx,
                         "usage: heartbeat <mcast> <serv> [ifname]");
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
