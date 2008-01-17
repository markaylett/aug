/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <csignal>
#include <iostream>

using namespace aug;
using namespace std;

namespace {

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

    void
    recvfrom(fdref ref, aug_netevent& ev, endpoint& ep)
    {
        char buf[AUG_NETEVENT_SIZE];
        aug::recvfrom(ref, buf, sizeof(buf), 0, ep);
        aug_unpacknetevent(&ev, buf);
    }

    size_t
    sendto(fdref ref, unsigned type, unsigned state, const endpoint& ep)
    {
        aug_netevent ev = { 0 };
        char buf[AUG_NETEVENT_SIZE];

        ev.type_ = type;
        ev.state_ = state;
        aug_packnetevent(buf, &ev);
        return aug::sendto(ref, buf, sizeof(buf), 0, ep);
    }

    class session {
        const char* const name_;
        fdref ref_;
        const endpoint& ep_;
        timer hbwait_;

    public:
        ~session() AUG_NOTHROW
        {
        }
        session(const char* name, fdref ref, const endpoint& ep, timers& ts)
            : name_(name),
              ref_(ref),
              ep_(ep),
              hbwait_(ts, null)
        {
            hbwait_.set(2000, *this);
        }
        void
        recvd(const aug_netevent& ev, const endpoint& from)
        {
        }
        void
        timercb(int id, unsigned& ms)
        {
            if (id == hbwait_.id()) {

                aug_info("hbint timeout");
                sendto(ref_, 1, 1, ep_);
            }
        }
    };

    void
    run(const char* name, fdref ref, const endpoint& ep)
    {
        muxer mux;
        timers ts;
        session s(name, ref, ep, ts);
        setfdeventmask(mux, ref, AUG_FDEVENTRD);

        timeval tv;
        int ret(!0);
        while (!stop_) {

            foreachexpired(ts, 0 == ret, tv);
            aug_info("timeout in: tv_sec=%d, tv_usec=%d", (int)tv.tv_sec,
                     (int)tv.tv_usec);

            while (AUG_RETINTR == (ret = waitfdevents(mux, tv)))
                ;

            aug_info("waitfdevents: %d", ret);

            if (0 < ret) {
                aug_netevent ev;
                endpoint from(null);
                recvfrom(ref, ev, from);
                s.recvd(ev, from);
            }
        }
    }
}

int
main(int argc, char* argv[])
{
    signal(SIGINT, sigcatch);

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);
        aug_setlogger(aug_daemonlogger);

        try {

            timeval tv;
            aug::gettimeofday(tv);

            if (argc < 4) {
                aug_error("usage: heartbeat <name> <mcast> <serv> [ifname]");
                return 1;
            }

            inetaddr in(argv[2]);
            smartfd sfd(aug::socket(family(in), SOCK_DGRAM));
            setreuseaddr(sfd, true);

            // Set outgoing multicast interface.

            if (5 == argc)
                setmcastif(sfd, argv[4]);

            // Don't receive packets from self.

            endpoint ep(inetany(family(in)), htons(atoi(argv[3])));
            aug::bind(sfd, ep);

            joinmcast(sfd, in, 5 == argc ? argv[4] : 0);
            setinetaddr(ep, in);
            run(argv[1], sfd, ep);

        } catch (const errinfo_error& e) {
            perrinfo(e, "aug::errorinfo_error");
        } catch (const exception& e) {
            aug_error("std::exception: %s", e.what());
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
