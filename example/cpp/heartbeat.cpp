#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    enum msgtype {
        CANDIDUP = 1,
        MASTERHB,
        MASTERUP,
        STANDDOWN,
        SLAVEHB,
        SLAVEUP,
        QUERY
    };

    enum state {
        CANDID = 2 << 8,
        MASTER = 1 << 8,
        SLAVE = 3 << 8
    };

    const char*
    tostring(msgtype src)
    {
        const char* s = 0;
        switch (src) {
        case CANDIDUP:
            s = "CANDIDUP";
            break;
        case MASTERHB:
            s = "MASTERHB";
            break;
        case MASTERUP:
            s = "MASTERUP";
            break;
        case STANDDOWN:
            s = "STANDDOWN";
            break;
        case SLAVEHB:
            s = "SLAVEHB";
            break;
        case SLAVEUP:
            s = "SLAVEUP";
            break;
        case QUERY:
            s = "QUERY";
            break;
        }
        return s;
    }

    const char*
    tostring(state src)
    {
        const char* s = 0;
        switch (src) {
        case CANDID:
            s = "CANDID";
            break;
        case MASTER:
            s = "MASTER";
            break;
        case SLAVE:
            s = "SLAVE";
            break;
        }
        return s;
    }

    msgtype
    recvfrom(fdref ref, endpoint& ep)
    {
        char ch;
        aug::recvfrom(ref, &ch, 1, 0, ep);
        return static_cast<msgtype>(ch);
    }

    size_t
    sendto(fdref ref, msgtype type, const endpoint& ep)
    {
        char ch(type);
        return aug::sendto(ref, &ch, 1, 0, ep);
    }

    class session : public timercb_base {
        fdref ref_;
        const endpoint& ep_;
        state state_;

        void
        sendup()
        {
            switch (state_) {
            case MASTER:
                sendto(ref_, MASTERUP, ep_);
                break;
            case CANDID:
                sendto(ref_, CANDIDUP, ep_);
                break;
            case SLAVE:
                sendto(ref_, SLAVEUP, ep_);
                break;
            }
        }

        void
        do_callback(idref ref, unsigned int& ms, struct aug_timers& timers)
        {
            aug_info("timer expired: ms='%d', current state='%s'", ms,
                     tostring(state_));
            switch (state_) {
            case MASTER:
                sendto(ref_, MASTERHB, ep_);
                break;
            case CANDID:
                sendto(ref_, MASTERUP, ep_);
                break;
            case SLAVE:
                sendto(ref_, SLAVEHB, ep_);
                break;
            }
        }

    public:
        ~session() NOTHROW
        {
        }
        session(fdref ref, const endpoint& ep)
            : ref_(ref),
              ep_(ep),
              state_(CANDID)
        {
            sendto(ref, CANDIDUP, ep);
        }
        void
        recv()
        {
            endpoint from(null);
            msgtype t(recvfrom(ref_, from));
            aug_info("message received: msgtype='%s', current state='%s'",
                     tostring(t), tostring(state_));

            // The following message types can be handled uniformly,
            // regardless of state.

            switch (t) {
            case SLAVEHB:
            case SLAVEUP:

                // No state change.
                break;

            case QUERY:

                // All nodes should respond to a query with an up message.

                sendup();
                break;

            default:
                break;
            }

            // Handle the other permutations.

            switch (t | state_) {

                // CANDID:
                // ------

            case CANDIDUP | CANDID:

                // Another candidate exists; ask them to stand-down.

                sendto(ref_, STANDDOWN, ep_);
                break;

            case MASTERHB | CANDID:
            case MASTERUP | CANDID:
            case STANDDOWN | CANDID:

                // Become a slave if a master already exists, or a stand-down
                // is received.

                sendto(ref_, SLAVEUP, ep_);
                break;

                // MASTER:
                // ------

            case CANDIDUP | MASTER:
            case MASTERHB | MASTER:
            case MASTERUP | MASTER:

                // Send a stand-down to the other master or candidate.

                sendto(ref_, STANDDOWN, ep_);
                break;

            case STANDDOWN | MASTER:

                // A master should become a slave if it is asked to
                // stand-down.

                sendto(ref_, SLAVEUP, ep_);
                break;

                // SLAVE:
                // -----

            case CANDIDUP | SLAVE:

                // Possible stand-down.
                break;

            case MASTERHB | SLAVE:
            case MASTERUP | SLAVE:
            case STANDDOWN | SLAVE:

                // No state change.
                break;
            }
        }
    };

    void
    run(fdref ref, const endpoint& ep)
    {
        mplexer mp;
        timers ts;
        timer t(ts, null);
        session s(ref, ep);
        t.set(2000, s);
        setioeventmask(mp, ref, AUG_IOEVENTRD);

        struct timeval tv;
        int ret(!0);
        for (int i(0); i < 5; ++i) {

            processtimers(ts, 0 == ret, tv);
            aug_info("tv_sec=%d, tv_usec=%d", (int)tv.tv_sec,
                     (int)tv.tv_usec);

            while (AUG_RETINTR == (ret = waitioevents(mp, tv)))
                ;

            aug_info("ret: %d", (int)ret);
            if (0 < ret)
                s.recv();
        }
    }
}

int
main(int argc, char* argv[])
{
    try {

        struct aug_errinfo errinfo;
        scoped_init init(errinfo);

        try {

            if (argc < 3) {
                aug_error("usage: heartbeat <mcast> <serv> [ifname]");
                return 1;
            }

            inetaddr in(argv[1]);
            smartfd sfd(aug::socket(family(in), SOCK_DGRAM));
            setreuseaddr(sfd, true);

            // Set outgoing multicast interface.

            if (4 == argc)
                setmcastif(sfd, argv[3]);

            // Don't receive packets from self.

            setmcastloop(sfd, false);

            endpoint ep(inetany(family(in)), htons(atoi(argv[2])));
            aug::bind(sfd, ep);

            joinmcast(sfd, in, 4 == argc ? argv[3] : 0);
            setinetaddr(ep, in);
            run(sfd, ep);

        } catch (const std::exception& e) {
            aug_perrinfo(e.what());
            return 1;
        }

    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
