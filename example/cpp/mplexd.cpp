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
    readconf(const char* conffile, bool daemon)
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

        mplexer& mplexer_;
        smartfd sfd_;
        timer timer_;
        buffer buffer_;
        int heartbeats_;

        ~session() AUG_NOTHROW
        {
            try {
                setioeventmask(mplexer_, sfd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(mplexer& mplexer, const smartfd& sfd, timers& timers)
            : mplexer_(mplexer),
              sfd_(sfd),
              timer_(timers, null),
              heartbeats_(0)
        {
            timer_.set(5000, *this);
        }
        void
        timercb(int id, unsigned& ms, aug_timers& timers)
        {
            ms = 0; // Cancel.

            aug_info("timeout");
            if (heartbeats_ < 3) {
                buffer_.putsome("heartbeat\n", 10);
                ++heartbeats_;
                setioeventmask(mplexer_, sfd_, AUG_IOEVENTRDWR);
            } else
                shutdown(sfd_, SHUT_RDWR);
        }
    };

    typedef smartptr<session> sessionptr;

    struct state {

        files files_;
        timers timers_;
        mplexer mplexer_;
        smartfd sfd_;
        map<int, sessionptr> sfds_;

        state(aug_filecb_t cb, const aug_var& var)
            : sfd_(null)
        {
            aug_hostserv hostserv;
            parsehostserv(address_, hostserv);

            endpoint ep(null);
            smartfd sfd(tcplisten(hostserv.host_, hostserv.serv_, ep));

            insertfile(files_, aug_eventin(), cb, var);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);

            insertfile(files_, sfd, cb, var);
            setioeventmask(mplexer_, sfd, AUG_IOEVENTRD);

            sfd_ = sfd;
        }
    };

    class service : public service_base {

        auto_ptr<state> state_;

        void
        setfdhook(fdref ref, unsigned short mask)
        {
            insertfile(state_->files_, ref, *this);
            try {
                setioeventmask(state_->mplexer_, ref, mask);
            } catch (...) {
                removefile(state_->files_, ref);
            }
        }

        bool
        readevent(int fd, aug_files& files)
        {
            aug_event event;
            AUG_DEBUG2("reading event");

            switch (aug::readevent(aug_eventin(), event).type_) {
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
            aug_destroyvar(&event.var_);
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
            setfdhook(sfd, AUG_IOEVENTRD);

            state_->sfds_.insert(make_pair
                                 (sfd.get(), sessionptr
                                  (new session(state_->mplexer_, sfd,
                                               state_->timers_))));
            return true;
        }

        bool
        connection(int fd, aug_files& files)
        {
            sessionptr ptr(state_->sfds_[fd]);
            unsigned short bits(ioevents(state_->mplexer_, fd));

            if (bits & AUG_IOEVENTRD) {

                AUG_DEBUG2("handling read event '%d'", fd);

                if (!ptr->buffer_.readsome(fd)) {

                    aug_info("closing connection '%d'", fd);
                    state_->sfds_.erase(fd);
                    return false;
                }

                setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRDWR);
                ptr->timer_.cancel();
                ptr->heartbeats_ = 0;
            }

            if (bits & AUG_IOEVENTWR) {

                if (!ptr->buffer_.writesome(fd)) {
                    setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRD);
                    ptr->timer_.reset(5000);
                }
            }

            return true;
        }

        const char*
        do_getopt(enum aug_option opt)
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
        do_readconf(const char* conffile, bool daemon)
        {
            test::readconf(conffile, daemon);
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            setsrvlogger("aug");

            aug_var var = { 0, this };
            state_.reset(new state(filememcb<service>, var));
        }

        void
        do_run()
        {
            timeval tv;

            aug_info("running daemon process");

            int ret(!0);
            while (!quit_) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    foreachexpired(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                foreachfile(state_->files_);
            }
        }

        void
        do_term()
        {
            aug_info("terminating daemon process");
            state_.reset();
        }

    public:
        ~service() AUG_NOTHROW
        {
        }
        bool
        filecb(int fd, aug_files& files)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            if (fd == aug_eventin())
                return readevent(fd, files);

            if (fd == state_->sfd_.get())
                return listener(fd, files);

            return connection(fd, files);
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
            aug_setloglevel(AUG_LOGINFO);
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
