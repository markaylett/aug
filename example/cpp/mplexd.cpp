/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

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
    cstring address_ = "127.0.0.1:8080";

    void
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
            readconf(conffile, aug::confcb<confcb>, null);
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
        buffer(unsigned size = 4096)
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
        readsome(fdref ref)
        {
            char buf[4096];
            size_t size(read(ref, buf, sizeof(buf) - 1));
            if (0 == size)
                return false;

            putsome(buf, size);
            return true;
        }
        bool
        writesome(fdref ref)
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

        muxer& muxer_;
        autosd sd_;
        timer timer_;
        buffer buffer_;
        int heartbeats_;

        ~session() AUG_NOTHROW
        {
            try {
                setfdeventmask(muxer_, sd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(muxer& muxer, autosd& sd, timers& timers)
            : muxer_(muxer),
              sd_(sd),
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
                setfdeventmask(muxer_, sd_, AUG_FDEVENTRDWR);
            } else
                shutdown(sd_, SHUT_RDWR);
        }
    };

    typedef smartptr<session> sessionptr;

    struct state {

        channels channels_;
        timers timers_;
        muxer muxer_;
        channelobptr serv_;
        map<int, sessionptr> sds_;

        state()
            : channels_(getmpool(aug_tlx)),
              serv_(null)
        {
            setfdeventmask(muxer_, aug_eventrd(), AUG_FDEVENTRD);

            aug_hostserv hostserv;
            parsehostserv(address_, hostserv);

            endpoint ep(null);
            autosd sd(tcpserver(hostserv.host_, hostserv.serv_, ep));
            setnonblock(sd, true);

            channelobptr serv(createserver(getmpool(aug_tlx), muxer_, sd));
            sd.release();

            insertchannel(channels_, serv);
            seteventmask(serv, AUG_FDEVENTRD);

            serv_ = serv;
        }
    };

    class service {

        auto_ptr<state> state_;

        void
        readevent()
        {
            AUG_CTXDEBUG2(aug_tlx, "reading event");

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
                return "Mark Aylett <mark@emantic.co.uk>";
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

        void
        readconf(const char* conffile, bool batch, bool daemon)
        {
            test::readconf(conffile, batch, daemon);
        }

        void
        init()
        {
            aug_ctxinfo(aug_tlx, "initialising daemon process");

            setsrvlogger("aug");
            state_.reset(new state());
        }

        void
        run()
        {
            timeval tv;

            aug_ctxinfo(aug_tlx, "running daemon process");

            int ret(!0);
            while (!quit_) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_FAILINTR == (ret = waitfdevents(state_
                                                               ->muxer_)))
                        ;

                } else {

                    foreachexpired(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_FAILINTR == (ret = waitfdevents(state_
                                                               ->muxer_, tv)))
                        ;
                }

                if (aug_fdevents(state_->muxer_, aug_eventrd()))
                    readevent();

                foreachchannel(state_->channels_, *this);
            }
        }

        void
        term()
        {
            aug_ctxinfo(aug_tlx, "terminating daemon process");
            state_.reset();
        }

        bool
        channelcb(unsigned id, streamobref streamob, unsigned short events)
        {
//             state_->sfds_.insert(make_pair
//                                  (sfd.get(), sessionptr
//                                   (new session(state_->muxer_, sfd,
//                                                state_->timers_))));

//             sessionptr ptr(state_->sds_[fd]);
//             unsigned short bits(fdevents(state_->muxer_, fd));

//             if (bits & AUG_FDEVENTRD) {

//                 AUG_DEBUG2("handling read event '%d'", fd);

//                 if (!ptr->buffer_.readsome(fd)) {

//                     aug_info("closing connection '%d'", fd);
//                     state_->sfds_.erase(fd);
//                     return false;
//                 }

//                 setfdeventmask(state_->muxer_, fd, AUG_FDEVENTRDWR);
//                 ptr->timer_.cancel();
//                 ptr->heartbeats_ = 0;
//             }

//             if (bits & AUG_FDEVENTWR) {

//                 if (!ptr->buffer_.writesome(fd)) {
//                     setfdeventmask(state_->muxer_, fd, AUG_FDEVENTRD);
//                     ptr->timer_.reset(5000);
//                 }
//             }

            return true;
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        aug_start();
        try {

            service serv;
            program_ = argv[0];

            blocksignals();
            return main(argc, argv, serv);

        } catch (const errinfo_error& e) {
            perrinfo(aug_tlx, "aug::errorinfo_error", e);
        } catch (const exception& e) {
            aug_ctxerror(aug_tlx, "std::exception: %s", e.what());
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
