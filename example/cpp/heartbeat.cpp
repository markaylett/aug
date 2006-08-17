#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    const unsigned HB_MS(2000);
    const unsigned LATENCY_MS(1000);
    const unsigned RESPONSE_MS(2000);
    const unsigned RAND_MS(1000);
    const unsigned MAX_NAME(32);

    enum msgtype {
        CANDIDUP = 1,
        HANDOVER,
        MASTERHB,
        MASTERUP,
        STANDDOWN,
        SLAVEHB,
        SLAVEUP,
        QUERY,
        SHUTDOWN
    };

    enum state {
        CANDID = 2 << 8,
        MASTER = 1 << 8,
        SLAVE = 3 << 8
    };

    struct packet {
        char name_[MAX_NAME];
        msgtype type_;
    };

    void
    fixedcpy(char* dst, const char* src, size_t size)
    {
        size_t i = aug_strlcpy(dst, src, size) + 1;
        if (i < size)
            memset(dst + i, '\0', size - i);
    }

    unsigned
    mwaitms()
    {
        return HB_MS + LATENCY_MS + (aug::rand() % RAND_MS);
    }

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
        case SHUTDOWN:
            s = "SHUTDOWN";
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

    void
    recvfrom(fdref ref, packet& p, endpoint& ep)
    {
        char buf[MAX_NAME + 1];
        aug::recvfrom(ref, buf, sizeof(buf), 0, ep);
        aug_strlcpy(p.name_, buf, sizeof(p.name_));
        p.type_ = (msgtype)buf[MAX_NAME];
    }

    size_t
    sendto(fdref ref, const char* name, msgtype type, const endpoint& ep)
    {
        char buf[MAX_NAME + 1];
        fixedcpy(buf, name, MAX_NAME);
        buf[MAX_NAME] = (char)type;
        return aug::sendto(ref, buf, sizeof(buf), 0, ep);
    }

    class session : private timercb_base {
        const char* const name_;
        fdref ref_;
        const endpoint& ep_;
        state state_;
        timer hbwait_;
        unsigned mwaitms_;
        timer mwait_;
        bool mseen_;
        struct timeval mlast_;
        endpoint slave_;

        void
        becomeslave()
        {
            aug_info("becoming slave");
            state_ = SLAVE;
            mwait_.reset(mwaitms_ = mwaitms());
            sendto(ref_, name_, SLAVEUP, ep_);
        }

        void
        sendup(const endpoint& ep)
        {
            switch (state_) {
            case MASTER:
                aug_info("broadcasting or sending master-up");
                sendto(ref_, name_, MASTERUP, ep);
                break;
            case CANDID:
                aug_info("broadcasting or sending candidate-up");
                sendto(ref_, name_, CANDIDUP, ep);
                break;
            case SLAVE:
                aug_info("broadcasting or sending slave-up");
                sendto(ref_, name_, SLAVEUP, ep);
                break;
            }
        }

        void
        do_callback(idref ref, unsigned& ms, struct aug_timers& timers)
        {
            if (ref == hbwait_) {

                aug_info("hbint timeout: state='%s'", tostring(state_));

                // Candidates for master will still heartbeat as slaves.

                if (MASTER == state_) {
                    aug_info("broadcasting master hb");
                    sendto(ref_, name_, MASTERHB, ep_);
                } else {
                    aug_info("broadcasting slave hb");
                    sendto(ref_, name_, SLAVEHB, ep_);
                }

            } else if (ref == mwait_) {

                aug_info("wait timeout: state='%s'", tostring(state_));

                if (CANDID == state_) {
                    aug_info("becoming master");
                    ms = 0; // Cancel timer.
                    state_ = MASTER;
                    sendto(ref_, name_, MASTERUP, ep_);
                } else {
                    aug_info("becoming candidate");
                    ms = RESPONSE_MS;
                    state_ = CANDID;
                    sendto(ref_, name_, CANDIDUP, ep_);
                }
            }
        }

    public:
        ~session() NOTHROW
        {
            // When a master shutsdown, if should send a handover message to
            // the last known slave.

            if (MASTER == state_ && null != slave_) {
                try {
                    aug_info("sending handover");
                    sendto(ref_, name_, HANDOVER, slave_);
                } AUG_PERRINFOCATCH;
            }

            sendto(ref_, name_, SHUTDOWN, slave_);
        }
        session(const char* name, fdref ref, const endpoint& ep, timers& ts)
            : name_(name),
              ref_(ref),
              ep_(ep),
              state_(SLAVE),
              hbwait_(ts, null),
              mwaitms_(mwaitms()),
              mwait_(ts, null),
              mseen_(false),
              slave_(null)
        {
            // When a node starts, it sends a query to determine the current
            // master (if any).

            aug_info("broadcasting slave-up and query");
            sendto(ref, name_, SLAVEUP, ep);
            sendto(ref, name_, QUERY, ep);

            hbwait_.set(HB_MS, *this);
            mwait_.set(mwaitms_, *this);
        }
        void
        recv()
        {
            packet p;
            endpoint from(null);
            recvfrom(ref_, p, from);
            if (0 == strcmp(name_, p.name_))
                return;

            aug_info("received: msgtype='%s', state='%s'", tostring(p.type_),
                     tostring(state_));

            // The following message types can be handled in a uniform manner,
            // regardless of the current state.

            switch (p.type_) {
            case HANDOVER:

                // Any node that receives a handover should instantly become a
                // master.  As an optimisation, a master node need not send an
                // up message.

                if (MASTER == state_) {
                    aug_info("becoming master");
                    state_ = MASTER;
                    sendto(ref_, name_, MASTERUP, ep_);
                }
                return;

            case SLAVEHB:
            case SLAVEUP:

                // Whenever a node receives a packet from a slave, the address
                // is stored so that a handover can be performed if need be.

                aug_info("storing slave address");
                slave_ = from;
                return;

            case MASTERHB:
            case MASTERUP:

                // Whenever a node receives a packet from a master, the time
                // is recorded.

                aug_info("storing time of master activity");
                mseen_ = true;
                aug::gettimeofday(mlast_);

                aug_info("resetting mwait timer");
                mwait_.reset(mwaitms_);

                break; // Other actions may still need to be performed.

            case QUERY:

                // All nodes should respond to a query by sending an up
                // message to the sender.

                aug_info("responding to query");
                sendup(from);
                return; // Done.

            default:
                break;
            }

            // Handle the other permutations.

            switch (state_ | p.type_) {

                // CANDID:
                // ------

            case CANDID | CANDIDUP:

                // When a candidate detects another candidate, it asks them to
                // stand-down.

                aug_info("sending stand-down");
                sendto(ref_, name_, STANDDOWN, from);
                break;

            case CANDID | MASTERHB:
            case CANDID | MASTERUP:
                //            case CANDID | SHUTDOWN:
            case CANDID | STANDDOWN:

                // A candidate becomes a slave either when it detects a master
                // or a stand-down is received.

                becomeslave();
                break;

                // MASTER:
                // ------

            case MASTER | CANDIDUP:
            case MASTER | MASTERHB:
            case MASTER | MASTERUP:

                // When a master detects either a candidate or another master,
                // it asks them to stand-down.

                aug_info("sending stand-down");
                sendto(ref_, name_, STANDDOWN, from);
                break;

                //            case MASTER | SHUTDOWN:
            case MASTER | STANDDOWN:

                // A master should become a slave if it is asked to
                // stand-down.

                becomeslave();
                break;

                // SLAVE:
                // -----

            case SLAVE | CANDIDUP:

                // If a slave detects a candidate, but is aware of a master,
                // it should ask the candidate to stand-down.

                if (mseen_) {
                    struct timeval tv;
                    gettimeofday(tv);
                    unsigned ms(tvtoms(tvsub(tv, mlast_)));
                    if (ms < HB_MS) {
                        aug_info("sending stand-down");
                        sendto(ref_, name_, STANDDOWN, from);
                    }
                }
                break;

            case SLAVE | MASTERHB:
            case SLAVE | MASTERUP:
                //            case SLAVE | SHUTDOWN:
            case SLAVE | STANDDOWN:

                // No state change.
                break;
            }
        }
    };

    void
    run(const char* name, fdref ref, const endpoint& ep)
    {
        mplexer mp;
        timers ts;
        session s(name, ref, ep, ts);
        setioeventmask(mp, ref, AUG_IOEVENTRD);

        struct timeval tv;
        int ret(!0);
        for (;;) {

            processtimers(ts, 0 == ret, tv);
            aug_info("tv_sec=%d, tv_usec=%d", (int)tv.tv_sec,
                     (int)tv.tv_usec);

            while (AUG_RETINTR == (ret = waitioevents(mp, tv)))
                ;

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

            struct timeval tv;
            aug::gettimeofday(tv);
            aug::srand(getpid() ^ tv.tv_sec ^ tv.tv_usec);

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

            //setmcastloop(sfd, false);

            endpoint ep(inetany(family(in)), htons(atoi(argv[3])));
            aug::bind(sfd, ep);

            joinmcast(sfd, in, 5 == argc ? argv[4] : 0);
            setinetaddr(ep, in);
            run(argv[1], sfd, ep);

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
