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

    const unsigned HB_MS(2000);
    const unsigned LATENCY_MS(1000);
    const unsigned RESPONSE_MS(2000);
    const unsigned RAND_MS(1000);
    const unsigned MAX_NAME(32);

    /*
      Heartbeat Protocol

      Member MB 0x001
      Candidate CD 0x002
      Leader LD 0x004
      Elect EL 0x010
      Join JN 0x020
      Leave LV 0x040
      Stand-down SD 0x080
      Status ST 0x100
      List LS 0x200

      NONE
      BELD: become LD
      BECD: become CD
      BEMB: become MB
      BRST: broadcast ST
      REMS: reset RESPONSE_MS
      RETO: reset TO
      SEEL: send EL
      SESD: send SD
      SEST: send ST
      WAIT: rand() wait
      CLIF: clear if last
      POIF: pop if last

      LD CD MB

      LDTO
      NONE (already LD)
      BELD
      BECD, REMS

      HBTO
      BRST
      BRST
      BRST

      LD & EL
      NONE (already LD)
      NONE
      BELD

      LD & (ST | JN)
      BEMB, RETO
      BEMB, RETO
      RETO

      LD & LV
      SEST (not req), CLIF
      SEST (not req), CLIF
      WAIT (election), CLIF

      LD & SD
      RETO, BEMB
      RETO, BEMB
      RETO

      LD & LS
      BEMB, SEST, RETO
      BEMB, SEST, RETO
      SEST, RETO

      CD & (ST | JN)
      SESD
      BEMB
      RETO

      CD & LV
      POIF
      POIF
      POIF

      CD & SD
      BEMB
      BEMB
      RETO

      CD & LS
      SESD, SEST
      BEMB, SEST
      RETO, SEST

      MB & (ST | JN)
      NONE
      NONE
      NONE

      MB & LV
      CLIF
      CLIF
      CLIF

      MB & SD
      BEMB
      BEMB
      NONE

      MB & LS
      SEST
      SEST
      SEST

      HBTO: heartbeat MS
      LDTO: HBTO + latency tolerance + rand()

      1.On stop, if LD and last is known then SEEL
      2.On start, broadcast (JN & LS)
      3.On stop, broadcast LV
      4.On HBTO, broadcast ST
      5.On receive, drop packets from self
      6.On state change, broadcast ST
      7.If not LV, update last known
      8.On receive, reset HBTO
      9.On conflict, send SD to node with least authority
    */

    /*
      LEAD 0x001
      CAND 0x002
      MEMB 0x004

      ELCT 0x010
      JOIN 0x020
      LEAV 0x040
      STDN 0x080
      STAT 0x100
      LIST 0x200
    */

    void
    ldwait()
    {
        /*
          LEAD: none - already leader
          CAND: become leader
          MEMB: become candidate: set RESPONSE_MS
         */
    }

    void
    hbwait()
    {
        /*
          ALL: broadcast status
         */
    }

    void
    leadelct()
    {
        /*
          LEAD: none - already leader
          CAND: become leader
          MEMB: become leader
         */
    }

    void
    leadjoin()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    leadleav()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    leadstdn()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    leadstat()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    leadlist()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candelct()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candjoin()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candleav()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candstdn()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candstat()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    candlist()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    membelct()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    membjoin()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    membleav()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    membstdn()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    membstat()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    void
    memblist()
    {
        /*
          LEAD:
          CAND:
          MEMB:
         */
    }

    enum msgtype {
        CANDIDUP = 1,
        HANDOVER,
        MASTERHB,
        MASTERUP,
        STANDDOWN,
        SLAVEHB,
        SLAVEUP,
        QUERY,
        NODEUP,
        NODEDOWN
    };

    enum state {
        CANDID = 2 << 8,
        MASTER = 1 << 8,
        SLAVE = 3 << 8
    };

    struct packet {
        char node_[MAX_NAME];
        msgtype type_;
    };

    volatile bool stop_ = false;

    void
    sigcatch(int sig)
    {
        stop_ = true;
    }

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
        case NODEUP:
            s = "NODEUP";
            break;
        case NODEDOWN:
            s = "NODEDOWN";
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
        aug_strlcpy(p.node_, buf, sizeof(p.node_));
        p.type_ = (msgtype)buf[MAX_NAME];
    }

    size_t
    sendto(fdref ref, const char* node, msgtype type, const endpoint& ep)
    {
        char buf[MAX_NAME + 1];
        fixedcpy(buf, node, MAX_NAME);
        buf[MAX_NAME] = (char)type;
        return aug::sendto(ref, buf, sizeof(buf), 0, ep);
    }

    class session {
        const char* const node_;
        fdref ref_;
        const endpoint& ep_;
        state state_;
        timer hbwait_;
        unsigned mwaitms_;
        timer mwait_;
        bool mseen_;
        timeval mlast_;
        endpoint slave_;

        void
        becomeslave()
        {
            aug_info("becoming slave");
            state_ = SLAVE;
            mwait_.reset(mwaitms_ = mwaitms());
            sendto(ref_, node_, SLAVEUP, ep_);
        }

        void
        sendup(const endpoint& ep)
        {
            switch (state_) {
            case MASTER:
                aug_info("broadcasting or sending master-up");
                sendto(ref_, node_, MASTERUP, ep);
                break;
            case CANDID:
                aug_info("broadcasting or sending candidate-up");
                sendto(ref_, node_, CANDIDUP, ep);
                break;
            case SLAVE:
                aug_info("broadcasting or sending slave-up");
                sendto(ref_, node_, SLAVEUP, ep);
                break;
            }
        }

    public:
        ~session() AUG_NOTHROW
        {
            // When a master shutsdown, if should send a handover message to
            // the last known slave.

            if (MASTER == state_ && null != slave_) {
                try {
                    aug_info("sending handover");
                    sendto(ref_, node_, HANDOVER, slave_);
                } AUG_PERRINFOCATCH;
            }

            aug_info("broadcasting node down");
            sendto(ref_, node_, NODEDOWN, ep_);
        }
        session(const char* node, fdref ref, const endpoint& ep, timers& ts)
            : node_(node),
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
            sendto(ref, node_, SLAVEUP, ep);
            sendto(ref, node_, QUERY, ep);

            hbwait_.set(HB_MS, *this);
            mwait_.set(mwaitms_, *this);
        }
        void
        recv(const packet& p, const endpoint& from)
        {
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
                    sendto(ref_, node_, MASTERUP, ep_);
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
            case NODEUP:

                // All nodes should respond to a query by sending an up
                // message to the sender.

                aug_info("responding to nodeup/query");
                sendup(from);
                return; // Done.

            case NODEDOWN:
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
                sendto(ref_, node_, STANDDOWN, from);
                break;

            case CANDID | MASTERHB:
            case CANDID | MASTERUP:
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
                sendto(ref_, node_, STANDDOWN, from);
                break;

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
                    timeval tv;
                    gettimeofday(tv);
                    unsigned ms(tvtoms(tvsub(tv, mlast_)));
                    if (ms < HB_MS) {
                        aug_info("sending stand-down");
                        sendto(ref_, node_, STANDDOWN, from);
                    }
                }
                break;

            case SLAVE | MASTERHB:
            case SLAVE | MASTERUP:
            case SLAVE | STANDDOWN:

                // No state change.
                break;
            }
        }
        void
        timercb(int id, unsigned& ms, aug_timers& timers)
        {
            if (id == hbwait_.id()) {

                aug_info("hbint timeout: state='%s'", tostring(state_));

                // Candidates for master will still heartbeat as slaves.

                if (MASTER == state_) {
                    aug_info("broadcasting master hb");
                    sendto(ref_, node_, MASTERHB, ep_);
                } else {
                    aug_info("broadcasting slave hb");
                    sendto(ref_, node_, SLAVEHB, ep_);
                }

            } else if (id == mwait_.id()) {

                aug_info("wait timeout: state='%s'", tostring(state_));

                if (CANDID == state_) {
                    aug_info("becoming master");
                    ms = 0; // Cancel timer.
                    state_ = MASTER;
                    sendto(ref_, node_, MASTERUP, ep_);
                } else {
                    aug_info("becoming candidate");
                    ms = RESPONSE_MS;
                    state_ = CANDID;
                    sendto(ref_, node_, CANDIDUP, ep_);
                }
            }
        }
    };

    void
    run(const char* node, fdref ref, const endpoint& ep)
    {
        mplexer mp;
        timers ts;
        session s(node, ref, ep, ts);
        setfdeventmask(mp, ref, AUG_FDEVENTRD);

        timeval tv;
        int ret(!0);
        while (!stop_) {

            foreachexpired(ts, 0 == ret, tv);
            aug_info("timeout in: tv_sec=%d, tv_usec=%d", (int)tv.tv_sec,
                     (int)tv.tv_usec);

            while (AUG_RETINTR == (ret = waitfdevents(mp, tv)))
                ;

            aug_info("waitfdevents: %d", ret);

            if (0 < ret) {
                packet p;
                endpoint from(null);
                recvfrom(ref, p, from);
                if (0 != strcmp(node, p.node_))
                    s.recv(p, from);
                else
                    aug_info("ignoring packet from self");
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
            aug::srand(getpid() ^ tv.tv_sec ^ tv.tv_usec);

            if (argc < 4) {
                aug_error("usage: heartbeat <node> <mcast> <serv> [ifname]");
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
