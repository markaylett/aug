/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsrvpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

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
            // On Windows, signal handlers are not called on the main thread.
            // The main thread's context will, therefore, be unavailble.
            abort();
        }
    }

    class server {
        chandler<server> chandler_;
        muxer muxer_;
        chans chans_;
        bool quit_;
        void
        readevent()
        {
            // Sticky events not required for fixed length blocking read.

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
            : muxer_(getmpool(aug_tlx)),
              chans_(null),
              quit_(false)
        {
            chandler_.reset(this);
            chans tmp(getmpool(aug_tlx), chandler_);
            chans_.swap(tmp);

            setmdeventmask(muxer_, rd_, AUG_MDEVENTRDEX);

            endpoint ep(null);
            autosd sd(tcpserver(host, serv, ep));
            setnonblock(sd, true);

            chanptr ob(createserver(getmpool(aug_tlx), muxer_, sd));
            sd.release();

            insertchan(chans_, ob);
        }
        smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_chandler>(id))
                return object_retain<aug_object>(chandler_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        aug_bool
        authchan_(unsigned id, const char* subject,
                  const char* issuer) AUG_NOTHROW
        {
            return AUG_TRUE;
        }
        void
        clearchan_(unsigned id) AUG_NOTHROW
        {
            aug_ctxinfo(aug_tlx, "clear connection");
        }
        void
        errorchan_(chanref chan, const aug_errinfo& errinfo) AUG_NOTHROW
        {
            // FIXME: implement.
        }
        aug_bool
        estabchan_(chanref chan, unsigned parent) AUG_NOTHROW
        {
            const unsigned id(getchanid(chan));

            aug_ctxinfo(aug_tlx, "id: %u", id);
            aug_ctxinfo(aug_tlx, "new connection");
            return AUG_TRUE;
        }
        aug_bool
        readychan_(chanref chan, unsigned short events) AUG_NOTHROW
        {
            const unsigned id(getchanid(chan));
            streamptr stream(object_cast<aug_stream>(chan));

            aug_ctxinfo(aug_tlx, "id: %u", id);
            if (events & AUG_MDEVENTRD) {
                char buf[1024];
                ssize_t n = read(stream, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    aug_ctxinfo(aug_tlx, "closing connection");
                    return AUG_FALSE;
                }
                buf[n] = '\0';
                aug_ctxinfo(aug_tlx, "data: %s", buf);
            }
            return AUG_TRUE;
        }
        void
        run()
        {
            while (!quit_) {

                try {

                    unsigned ready(getreadychans(chans_));
                    if (ready) {

                        // Some are ready so don't block.

                        pollmdevents(muxer_);

                    } else {

                        // No timers so wait indefinitely.

                        scoped_unblock unblock;
                        waitmdevents(muxer_);
                    }

                } catch (const intr_exception&) {
                    continue;
                }

                // Sticky events not required for fixed length blocking read.

                if (getmdevents(muxer_, rd_))
                    readevent();

                aug_ctxinfo(aug_tlx, "before");
                dumpchans(chans_);
                processchans(chans_);
                dumpchans(chans_);
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

        setsighandler(sighandler);

        scoped_block block;
        server serv(argv[1], argv[2]);

        serv.run();
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
