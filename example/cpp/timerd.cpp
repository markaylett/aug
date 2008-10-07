/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrvpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>
#include <memory>

#include <time.h>

using namespace aug;
using namespace aug;
using namespace std;

namespace test {

    const char* program_;

    char conffile_[AUG_PATH_MAX + 1] = "";
    char rundir_[AUG_PATH_MAX + 1] = "";
    char pidfile_[AUG_PATH_MAX + 1] = "timerd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "timerd.log";

    aug_result
    confcb(void* arg, const char* name, const char* value)
    {
        if (0 == aug_strcasecmp(name, "loglevel")) {

            unsigned level(strtoui(value, 10));
            aug_ctxinfo(aug_tlx, "setting log level: %d", level);
            aug_setloglevel(aug_tlx, level);

        } else if (0 == aug_strcasecmp(name, "logfile")) {

            aug_strlcpy(logfile_, value, sizeof(logfile_));

        } else if (0 == aug_strcasecmp(name, "pidfile")) {

            aug_strlcpy(pidfile_, value, sizeof(pidfile_));

        } else if (0 == aug_strcasecmp(name, "rundir")) {

            // Once set, the run directory should not change.

            if (!*rundir_)
                realpath(rundir_, value, sizeof(rundir_));

        } else {

            throw runtime_error("option not supported");
        }

        return AUG_SUCCESS;
    }

    class service {

        struct state {
            muxer muxer_;
            timers timers_;
            timer timer_;
        public:
            state()
                : muxer_(getmpool(aug_tlx)),
                  timers_(getmpool(aug_tlx)),
                  timer_(timers_, null)
            {
            }
        };

        bool daemon_;
        int remain_;
        auto_ptr<state> state_;

        void
        readevent()
        {
            int fd(aug_eventrd());
            AUG_CTXDEBUG2(aug_tlx, "checking event pipe '%d'", fd);

            if (!getmdevents(state_->muxer_, fd))
                return;

            AUG_CTXDEBUG2(aug_tlx, "reading event");

            // Sticky events not required for fixed length blocking read.

            pair<int, smartob<aug_object> > event(aug::readevent(fd));

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
                remain_ = 0;
                break;
            }
        }

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

    public:
        ~service() AUG_NOTHROW
        {
        }

        service()
            : daemon_(false),
              remain_(5)
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
                return "Timer Daemon";
            case AUG_OPTPIDFILE:
                return pidfile_;
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "timerd";
            }
            return 0;
        }

        aug_result
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
            return AUG_SUCCESS;
        }

        aug_result
        init()
        {
            aug_ctxinfo(aug_tlx, "initialising daemon process");

            verify(aug_setsrvlogger("aug"));

            auto_ptr<state> ptr(new state());
            setmdeventmask(ptr->muxer_, aug_eventrd(), AUG_MDEVENTRD);
            state_ = ptr;
            return AUG_SUCCESS;
        }

        aug_result
        run()
        {
            timeval tv;

            aug_ctxinfo(aug_tlx, "running daemon process");

            state_->timer_.set(5000, *this);

            unsigned ready(!0);
            while (0 < remain_) {

                processexpired(state_->timers_, 0 == ready, tv);

                try {

                    scoped_unblock unblock;

                    if (state_->timers_.empty()) {

                        // No timers so wait indefinitely.

                        ready = waitmdevents(state_->muxer_);

                    } else {

                        // Wait upto next timer expiry.

                        ready = waitmdevents(state_->muxer_, tv);
                    }

                } catch (const intr_exception&) {
                    ready = !0; // Not timeout.
                    continue;
                }

                // Sticky events not required for fixed length blocking read.

                readevent();
            }
            return AUG_SUCCESS;
        }

        void
        term()
        {
            aug_ctxinfo(aug_tlx, "terminating daemon process");
            state_.reset();
        }

        void
        timercb(int id, unsigned& ms)
        {
            aug_ctxinfo(aug_tlx, "timer fired");
            --remain_;
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        autobasictlx();
		service serv;

        program_ = argv[0];

        return main(argc, argv, serv);

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 1; // aug_main() does not return.
}
