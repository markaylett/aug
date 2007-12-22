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

using namespace aub;
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
            aug_info("setting log level: %d", level);
            aug_setloglevel(level);

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

        aug_info("run directory: %s", rundir_);
        aug_info("pid file: %s", pidfile_);
        aug_info("log file: %s", logfile_);
        aug_info("log level: %d", aug_loglevel());
    }

    void
    readconf(const char* conffile, bool batch, bool daemon)
    {
        if (conffile) {
            aug_info("reading: %s", conffile);
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
        smartfd sfd_;
        timer timer_;
        buffer buffer_;
        int heartbeats_;

        ~session() AUG_NOTHROW
        {
            try {
                setfdeventmask(muxer_, sfd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(muxer& muxer, const smartfd& sfd, timers& timers)
            : muxer_(muxer),
              sfd_(sfd),
              timer_(timers, null),
              heartbeats_(0)
        {
            timer_.set(5000, *this);
        }
        void
        timercb(int id, unsigned& ms)
        {
            ms = 0; // Cancel.

            aug_info("timeout");
            if (heartbeats_ < 3) {
                buffer_.putsome("heartbeat\n", 10);
                ++heartbeats_;
                setfdeventmask(muxer_, sfd_, AUG_FDEVENTRDWR);
            } else
                shutdown(sfd_, SHUT_RDWR);
        }
    };

    typedef smartptr<session> sessionptr;

    struct state {

        files files_;
        timers timers_;
        muxer muxer_;
        smartfd sfd_;
        map<int, sessionptr> sfds_;

        state(aug_filecb_t cb, obref<aub_object> ob)
            : sfd_(null)
        {
            aug_hostserv hostserv;
            parsehostserv(address_, hostserv);

            endpoint ep(null);
            smartfd sfd(tcplisten(hostserv.host_, hostserv.serv_, ep));

            insertfile(files_, aug_eventrd(), cb, ob);
            setfdeventmask(muxer_, aug_eventrd(), AUG_FDEVENTRD);

            insertfile(files_, sfd, cb, ob);
            setfdeventmask(muxer_, sfd, AUG_FDEVENTRD);

            sfd_ = sfd;
        }
    };

    class service {

        auto_ptr<state> state_;

        void
        setfdhook(fdref ref, unsigned short mask)
        {
            insertfile(state_->files_, ref, *this);
            try {
                setfdeventmask(state_->muxer_, ref, mask);
            } catch (...) {
                removefile(state_->files_, ref);
            }
        }

        bool
        readevent(int fd, aug_files& files)
        {
            AUG_DEBUG2("reading event");

            pair<int, smartob<aub_object> >
                event(aug::readevent(aug_eventrd()));

            switch (event.first) {
            case AUG_EVENTRECONF:
                aug_info("received AUG_EVENTRECONF");
                if (*conffile_) {
                    aug_info("reading: %s", conffile_);
                    aug::readconf(conffile_, aug::confcb<confcb>, null);
                }
                reconf();
                break;
            case AUG_EVENTSTATUS:
                aug_info("received AUG_EVENTSTATUS");
                break;
            case AUG_EVENTSTOP:
                aug_info("received AUG_EVENTSTOP");
                quit_ = true;
                break;
            }
            return true;
        }

        bool
        listener(int fd, aug_files& files)
        {
            aug_endpoint ep;

            AUG_DEBUG2("accepting connection");

            smartfd sfd(null);
            try {

                sfd = accept(state_->sfd_, ep);

            } catch (const errinfo_error& e) {

                if (aug_acceptlost()) {
                    aug_warn("accept() failed: %s", e.what());
                    return true;
                }
                throw;
            }

            aug_info("initialising connection '%d'", sfd.get());

            setnodelay(sfd, true);
            setnonblock(sfd, true);
            setfdhook(sfd, AUG_FDEVENTRD);

            state_->sfds_.insert(make_pair
                                 (sfd.get(), sessionptr
                                  (new session(state_->muxer_, sfd,
                                               state_->timers_))));
            return true;
        }

        bool
        connection(int fd, aug_files& files)
        {
            sessionptr ptr(state_->sfds_[fd]);
            unsigned short bits(fdevents(state_->muxer_, fd));

            if (bits & AUG_FDEVENTRD) {

                AUG_DEBUG2("handling read event '%d'", fd);

                if (!ptr->buffer_.readsome(fd)) {

                    aug_info("closing connection '%d'", fd);
                    state_->sfds_.erase(fd);
                    return false;
                }

                setfdeventmask(state_->muxer_, fd, AUG_FDEVENTRDWR);
                ptr->timer_.cancel();
                ptr->heartbeats_ = 0;
            }

            if (bits & AUG_FDEVENTWR) {

                if (!ptr->buffer_.writesome(fd)) {
                    setfdeventmask(state_->muxer_, fd, AUG_FDEVENTRD);
                    ptr->timer_.reset(5000);
                }
            }

            return true;
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
            aug_info("initialising daemon process");

            setsrvlogger("aug");
            smartob<aug_addrob> ob(createaddrob(this, 0));
            state_.reset(new state(filememcb<service>, ob));
        }

        void
        run()
        {
            timeval tv;

            aug_info("running daemon process");

            int ret(!0);
            while (!quit_) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitfdevents(state_
                                                              ->muxer_)))
                        ;

                } else {

                    foreachexpired(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitfdevents(state_
                                                              ->muxer_,
                                                              tv)))
                        ;
                }

                foreachfile(state_->files_);
            }
        }

        void
        term()
        {
            aug_info("terminating daemon process");
            state_.reset();
        }

        bool
        filecb(int fd)
        {
            if (!fdevents(state_->muxer_, fd))
                return true;

            if (fd == aug_eventrd())
                return readevent(fd, state_->files_);

            if (fd == state_->sfd_.get())
                return listener(fd, state_->files_);

            return connection(fd, state_->files_);
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);
        try {

            service serv;
            program_ = argv[0];

            blocksignals();
            return main(argc, argv, serv);

        } catch (const errinfo_error& e) {
            perrinfo(e, "aug::errorinfo_error");
        } catch (const exception& e) {
            aug_error("std::exception: %s", e.what());
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
