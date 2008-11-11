/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrvpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <vector>

#include <time.h>

using namespace aug;
using namespace aug;
using namespace std;

namespace test {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;

    cstring conffile_= "";
    cstring rundir_ = "";
    cstring pidfile_ = "mplexd.pid";
    cstring logfile_ = "mplexd.log";
    cstring address_ = "0.0.0.0:44308";

    aug_result
    confcb(void* arg, const char* name, const char* value)
    {
        if (0 == aug_strcasecmp(name, "address")) {

            aug_strlcpy(address_, value, sizeof(address_));

        } else if (0 == aug_strcasecmp(name, "loglevel")) {

            unsigned level(strtoui(value, 10));
            aug_ctxinfo(aug_tlx, "setting log level: %d", level);
            setloglevel(aug_tlx, level);

        } else if (0 == aug_strcasecmp(name, "logfile")) {

            aug_strlcpy(logfile_, value, sizeof(logfile_));

        } else if (0 == aug_strcasecmp(name, "pidfile")) {

            aug_strlcpy(pidfile_, value, sizeof(pidfile_));

        } else if (0 == aug_strcasecmp(name, "rundir")) {

            // Once set, the run directory should not change.

            if (!*rundir_)
                realpath(rundir_, value, sizeof(rundir_));

        } else {

            string s("option not supported: ");
            s += name;
            throw runtime_error("option not supported");
        }

        return AUG_SUCCESS;
    }

    bool daemon_(false);
    bool quit_(false);

    void
    reconf()
    {
        aug::chdir(rundir_);

        if (daemon_)
            openlog(logfile_);

        aug_ctxinfo(aug_tlx, "run directory: %s", rundir_);
        aug_ctxinfo(aug_tlx, "pid file: %s", pidfile_);
        aug_ctxinfo(aug_tlx, "log file: %s", logfile_);
        aug_ctxinfo(aug_tlx, "log level: %d", aug_loglevel());
    }

    void
    readconf(const char* conffile, bool batch, bool daemon)
    {
        if (conffile) {
            aug_ctxinfo(aug_tlx, "reading: %s", conffile);
			aug::readconf(conffile, aug::confcb<confcb>, null);
            aug_strlcpy(conffile_, conffile, sizeof(conffile_));
        }

        daemon_ = daemon;

        // Use working directory as default.

        if (!*rundir_)
            realpath(rundir_, getcwd().c_str(), sizeof(rundir_));

        reconf();
    }

    class buffer {
        vector<char> vec_;
        size_t begin_, end_;
    public:
        explicit
        buffer(unsigned size = AUG_BUFSIZE)
            : vec_(size),
              begin_(0),
              end_(0)
        {
        }
        void
        putsome(const void* buf, size_t len)
        {
            if (vec_.size() - end_ < len)
                vec_.resize(end_ + len);

            memcpy(&vec_[end_], buf, len);
            end_ += len;
        }
        bool
        readsome(streamref ref)
        {
            char buf[AUG_BUFSIZE];
            size_t size(read(ref, buf, sizeof(buf) - 1));
            if (0 == size)
                return false;

            putsome(buf, size);
            return true;
        }
        bool
        writesome(streamref ref)
        {
            size_t size(end_ - begin_);
            size = write(ref, &vec_[begin_], size);
            if ((begin_ += size) == end_) {
                begin_ = end_ = 0;
                return false;
            }
            return true;
        }
        bool
        empty() const
        {
            return begin_ == end_;
        }
    };

    struct session {

        chanptr chan_;
        timer timer_;
        buffer buffer_;
        int heartbeats_;

        session(const chanref& chan, timers& timers)
            : chan_(object_retain(chan)),
              timer_(timers, null),
              heartbeats_(0)
        {
            timer_.set(5000, *this);
        }
        void
        timercb(int id, unsigned& ms)
        {
            ms = 0; // Cancel.

            aug_ctxinfo(aug_tlx, "timeout");
            if (heartbeats_ < 3) {
                buffer_.putsome("heartbeat\n", 10);
                ++heartbeats_;
                setchanwantwr(chan_, AUG_TRUE);
            } else {
                streamptr strm(object_cast<aug_stream>(chan_));
                shutdown(strm);
            }
        }
    };

    typedef smartptr<session> sessionptr;

    struct state {

        chandler<state> chandler_;
        map<unsigned, sessionptr> sessions_;
        muxer muxer_;
        timers timers_;
        chans chans_;

        state()
            : muxer_(getmpool(aug_tlx)),
              timers_(getmpool(aug_tlx)),
              chans_(null)
        {
            chandler_.reset(this);
            chans tmp(getmpool(aug_tlx), chandler_);
            chans_.swap(tmp);

            setmdeventmask(muxer_, aug_eventrd(), AUG_MDEVENTRDEX);

            aug_hostserv hostserv;
            parsehostserv(address_, hostserv);

            endpoint ep(null);
            autosd sd(tcpserver(hostserv.host_, hostserv.serv_, ep));
            setnonblock(sd, true);

            chanptr serv(createserver(getmpool(aug_tlx), muxer_, sd));
            sd.release();

            insertchan(chans_, serv);
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
            aug_ctxinfo(aug_tlx, "clearing connection");
            sessions_.erase(id);
        }
        void
        errorchan_(chanref chan, const aug_errinfo& errinfo) AUG_NOTHROW
        {
            // FIXME: implement.
        }
        aug_bool
        estabchan_(chanref chan, unsigned parent) AUG_NOTHROW
        {
            aug_ctxinfo(aug_tlx, "inserting connection");
            sessions_.insert(make_pair(getchanid(chan), sessionptr
                                       (new session(chan, timers_))));
            return AUG_TRUE;
        }
        aug_bool
        readychan_(chanref chan, unsigned short events) AUG_NOTHROW
        {
            const unsigned id(getchanid(chan));
            streamptr stream(object_cast<aug_stream>(chan));
            sessionptr sess(sessions_[id]);

            if (events & AUG_MDEVENTRDEX) {

                AUG_CTXDEBUG2(aug_tlx, "handling read event [%d]", id);

                if (!sess->buffer_.readsome(stream)) {

                    aug_ctxinfo(aug_tlx, "closing connection [%d]", id);
                    return AUG_FALSE;
                }

                setchanwantwr(chan, AUG_TRUE);
                sess->timer_.cancel();
                sess->heartbeats_ = 0;
            }

            if (events & AUG_MDEVENTWR) {

                if (!sess->buffer_.writesome(stream)) {
                    setchanwantwr(chan, AUG_FALSE);
                    sess->timer_.reset(5000);
                }
            }

            return AUG_TRUE;
        }
    };

    class service {

        auto_ptr<state> state_;

        void
        readevent()
        {
            AUG_CTXDEBUG2(aug_tlx, "reading event");

            // Sticky events not required for fixed length blocking read.

            pair<int, smartob<aug_object> >
                event(aug::readevent(aug_eventrd()));

            switch (event.first) {
            case AUG_EVENTRECONF:
                aug_ctxinfo(aug_tlx, "received AUG_EVENTRECONF");
                if (*conffile_) {
                    aug_ctxinfo(aug_tlx, "reading: %s", conffile_);
                    aug::readconf(conffile_, aug::confcb<confcb>, null);
                }
                reconf();
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
        ~service() AUG_NOTHROW
        {
        }

        const char*
        getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTEMAIL:
                return "Mark Aylett <mark.aylett@gmail.com>";
            case AUG_OPTLONGNAME:
                return "Multiplexer Daemon";
            case AUG_OPTPIDFILE:
                return pidfile_;
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "mplexd";
            }
            return 0;
        }

        aug_result
        readconf(const char* conffile, bool batch, bool daemon)
        {
            test::readconf(conffile, batch, daemon);
            return AUG_SUCCESS;
        }

        aug_result
        init()
        {
            aug_ctxinfo(aug_tlx, "initialising daemon process");

            setsrvlogger("aug");
            state_.reset(new state());
            return AUG_SUCCESS;
        }

        aug_result
        run()
        {
            timeval tv;

            aug_ctxinfo(aug_tlx, "running daemon process");

            unsigned ready(!0);
            while (!quit_) {

                processexpired(state_->timers_, 0 == ready, tv);

                try {

                    ready = getreadychans(state_->chans_);
                    if (ready) {

                        // Channels ready so don't wait.

                        pollmdevents(state_->muxer_);

                    } else if (state_->timers_.empty()) {

                        // No timers so wait indefinitely.

                        scoped_unblock unblock;
                        ready = waitmdevents(state_->muxer_);

                    } else {

                        // Wait upto next timer expiry.

                        scoped_unblock unblock;
                        ready = waitmdevents(state_->muxer_, tv);
                    }

                } catch (const intr_exception&) {
                    ready = !0; // Not timeout.
                    continue;
                }

                // Sticky events not required for fixed length blocking read.

                if (getmdevents(state_->muxer_, aug_eventrd()))
                    readevent();

                processchans(state_->chans_);
            }

            return AUG_SUCCESS;
        }

        void
        term()
        {
            aug_ctxinfo(aug_tlx, "terminating daemon process");
            state_.reset();
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {
        autobasictlx();
        setloglevel(aug_tlx, AUG_LOGDEBUG0 + 3);

        service serv;
        program_ = argv[0];

        blocksignals();
        return main(argc, argv, serv);

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
