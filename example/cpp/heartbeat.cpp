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
        HANDOVER,
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
        case HANDOVER:
            s = "HANDOVER";
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
        bool mseen_;
        struct timeval mlast_;
        endpoint slave_;

        void
        sendup(const endpoint& ep)
        {
            switch (state_) {
            case MASTER:
                sendto(ref_, MASTERUP, ep);
                break;
            case CANDID:
                sendto(ref_, CANDIDUP, ep);
                break;
            case SLAVE:
                sendto(ref_, SLAVEUP, ep);
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
            // When a master shutsdown, if should send a handover message to
            // the last known slave.

            if (MASTER == state_ && null != slave_) {
                try {
                    sendto(ref_, HANDOVER, slave_);
                } AUG_PERRINFOCATCH;
            }
        }
        session(fdref ref, const endpoint& ep)
            : ref_(ref),
              ep_(ep),
              state_(CANDID),
              mseen_(false),
              slave_(null)
        {
            // When a node starts, it sends a query to determine the current
            // master (if any).

            sendto(ref, QUERY, ep);
        }
        void
        recv()
        {
            endpoint from(null);
            msgtype t(recvfrom(ref_, from));
            aug_info("message received: msgtype='%s', current state='%s'",
                     tostring(t), tostring(state_));

            // The following message types can be handled in a uniform manner,
            // regardless of the current state.

            switch (t) {
            case HANDOVER:

                // Any node that receives a handover should instantly become a
                // master.  As an optimisation, a master node need not send an
                // up message.

                if (MASTER == state_) {
                    state_ = MASTER;
                    sendto(ref_, MASTERUP, ep_);
                }
                return;

            case SLAVEHB:
            case SLAVEUP:

                // Whenever a node receives a packet from a slave, the address
                // is stored so that a handover can be performed if need be.

                slave_ = from;
                return;

            case MASTERHB:
            case MASTERUP:

                // Whenever a node receives a packet from a master, the time
                // is recorded.

                mseen_ = true;
                aug::gettimeofday(mlast_);

                break; // Other actions may still need to be performed.

            case QUERY:

                // All nodes should respond to a query by sending an up
                // message to the sender.

                sendup(from);
                return; // Done.

            default:
                break;
            }

            // Handle the other permutations.

            switch (state_ | t) {

                // CANDID:
                // ------

            case CANDID | CANDIDUP:

                // When a candidate detects another candidate, it asks them to
                // stand-down.

                sendto(ref_, STANDDOWN, from);
                break;

            case CANDID | MASTERHB:
            case CANDID | MASTERUP:
            case CANDID | STANDDOWN:

                // A candidate becomes a slave either when it detects a master
                // or a stand-down is received.

                state_ = SLAVE;
                sendto(ref_, SLAVEUP, ep_);
                break;

                // MASTER:
                // ------

            case MASTER | CANDIDUP:
            case MASTER | MASTERHB:
            case MASTER | MASTERUP:

                // When a master detects either a candidate or another master,
                // it asks them to stand-down.

                sendto(ref_, STANDDOWN, from);
                break;

            case MASTER | STANDDOWN:

                // A master should become a slave if it is asked to
                // stand-down.

                state_ = SLAVE;
                sendto(ref_, SLAVEUP, ep_);
                break;

                // SLAVE:
                // -----

            case SLAVE | CANDIDUP:

                // If a slave detects a candidate, but is aware of a master,
                // it should ask the candidate to stand-down.

                if (mseen_) {
                    struct timeval tv;
                    gettimeofday(tv);
                    unsigned int ms(tvtoms(tvsub(tv, mlast_)));
                    if (ms < 2000)
                        sendto(ref_, STANDDOWN, from);
                }
                break;

            case SLAVE | MASTERHB:
            case SLAVE | MASTERUP:
            case SLAVE | STANDDOWN:

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
