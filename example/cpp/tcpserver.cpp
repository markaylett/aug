/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrvpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    automd rd_(null), wr_(null);

    void
    sighandler(int sig)
    {
        aug_event event;
        try {
            writeevent(wr_, setsigevent(event, sig));
        } catch (...) {
            abort();
        }
    }

    class server {
        muxer muxer_;
        channels channels_;
        channelobptr serv_;
        bool quit_;
        void
        readevent()
        {
            pair<int, smartob<aug_object> > event(aug::readevent(rd_));

            switch (event.first) {
            case AUG_EVENTRECONF:
                aug_ctxinfo(aug_tlx, "received AUG_EVENTRECONF");
                break;
            case AUG_EVENTSTATUS:
                aug_ctxinfo(aug_tlx, "received AUG_EVENTSTATUS");
                break;
            case AUG_EVENTSTOP:
                aug_ctxinfo(aug_tlx, "received AUG_EVENTSTOP");
                quit_ = true;
                break;
            }
        }
    public:
        server(const char* host, const char* serv)
            : channels_(getmpool(aug_tlx)),
              serv_(null),
              quit_(false)
        {
            setfdeventmask(muxer_, rd_, AUG_FDEVENTRD);

            endpoint ep(null);
            autosd sd(tcpserver(host, serv, ep));
            setnonblock(sd, true);

            channelobptr ob(createserver(getmpool(aug_tlx), muxer_, sd));
            sd.release();

            insertchannel(channels_, ob);
        }
        bool
        channelcb(unsigned id, streamobref streamob, unsigned short events)
        {
            aug_ctxinfo(aug_tlx, "id: %u", id);
            if (0 == events) {
                aug_ctxinfo(aug_tlx, "new connection");
            } else if (events & AUG_FDEVENTRD) {
                char buf[1024];
                ssize_t n = read(streamob, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    aug_ctxinfo(aug_tlx, "closing connection");
                    return false;
                }
                buf[n] = '\0';
                aug_ctxinfo(aug_tlx, "data: %s", buf);
            }
            return true;
        }
        void
        run()
        {
            while (!quit_) {
                {
                    scoped_unblock unblock;
                    while (AUG_FAILINTR == waitfdevents(muxer_))
                        ;
                }

                if (fdevents(muxer_, rd_))
                    readevent();

                aug_ctxinfo(aug_tlx, "before");
                dumpchannels(channels_);
                foreachchannel(channels_, *this);
                dumpchannels(channels_);
                aug_ctxinfo(aug_tlx, "after");
            }
        }
    };
}

int
main(int argc, char* argv[])
{
    try {

        autobasictlx();
        setloglevel(aug_tlx, AUG_LOGDEBUG0 + 3);

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: tcpserver <host> <serv>");
            return 1;
        }

        autosds sds(muxerpipe());
        rd_ = sds[0];
        wr_ = sds[1];

        signalhandler(sighandler);

        scoped_block block;
        server serv(argv[1], argv[2]);
        serv.run();
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
