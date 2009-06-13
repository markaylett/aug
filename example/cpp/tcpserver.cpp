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

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    events events_(null);

    void
    sighandler(int sig)
    {
        aug_event event;
        try {
            writeevent(events_, sigtoevent(sig, event));
        } catch (const block_exception&) {
            // Event pipe is full.
        } catch (...) {
            // On Windows, signal handlers are not called on the main thread.
            // The main thread's context will, therefore, be unavailble.
            abort();
        }
    }

    class server : public mpool_ops {
        chandler<server> chandler_;
        muxer muxer_;
        chans chans_;
        bool quit_;
        void
        readevent()
        {
            // Sticky events not required for fixed length blocking read.

            pair<int, objectptr> event(aug::readevent(events_));

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

            setmdeventmask(muxer_, eventsmd(), AUG_MDEVENTRDEX);

            endpoint ep(null);
            autosd sd(tcpserver(host, serv, ep));
            setnonblock(sd, true);

            chanptr ob(createserver(getmpool(aug_tlx), muxer_, sd));
            sd.release();

            insertchan(chans_, ob);
        }
        objectptr
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

                        scoped_sigunblock unblock;
                        waitmdevents(muxer_);
                    }

                } catch (const intr_exception&) {
                    continue;
                }

                // Sticky events not required for fixed length blocking read.

                if (getmdevents(muxer_, eventsmd()))
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

        autotlx();
        setloglevel(aug_tlx, AUG_LOGDEBUG0 + 3);

        if (argc < 3) {
            aug_ctxerror(aug_tlx, "usage: tcpserver <host> <serv>");
            return 1;
        }

        events local(getmpool(aug_tlx));
        events_.swap(local);

        scoped_sighandler handler(sighandler);
        scoped_sigblock block;
        server serv(argv[1], argv[2]);

        serv.run();
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1;
}
