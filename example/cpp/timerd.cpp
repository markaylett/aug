/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <memory>
#include <time.h>

using namespace aug;
using namespace std;

namespace test {

    const char* program_;

    char conffile_[AUG_PATH_MAX + 1] = "";
    char rundir_[AUG_PATH_MAX + 1] = "";
    char pidfile_[AUG_PATH_MAX + 1] = "timerd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "timerd.log";

    class confcb : public confcb_base {
        void
        do_callback(const char* name, const char* value)
        {
            if (0 == aug_strcasecmp(name, "loglevel")) {

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

                throw runtime_error("option not supported");
            }
        }
    };

    class service : public service_base, public timercb_base {

        struct state {
            mplexer mplexer_;
            timers timers_;
            timer timer_;
        public:
            state()
                : timer_(timers_, null)
            {
            }
        };

        bool daemon_;
        int remain_;
        auto_ptr<state> state_;

        void
        readevent()
        {
            int fd(aug_eventin());
            aug_event event;

            AUG_DEBUG2("checking event pipe '%d'", fd);

            if (!ioevents(state_->mplexer_, fd))
                return;

            AUG_DEBUG2("reading event");

            switch (aug::readevent(aug_eventin(), event).type_) {
            case AUG_EVENTRECONF:
                aug_info("received AUG_EVENTRECONF");
                if (*conffile_) {
                    aug_info("reading: %s", conffile_);
                    test::confcb cb;
                    aug::readconf(conffile_, cb);
                }
                reconf();
                break;
            case AUG_EVENTSTATUS:
                aug_info("received AUG_EVENTSTATUS");
                break;
            case AUG_EVENTSTOP:
                aug_info("received AUG_EVENTSTOP");
                remain_ = 0;
                break;
            }
            aug_destroyvar(&event.var_);
        }

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

        const char*
        do_getopt(enum aug_option opt)
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

        void
        do_readconf(const char* conffile, bool daemon)
        {
            if (conffile) {
                aug_info("reading: %s", conffile);
                test::confcb cb;
                aug::readconf(conffile, cb);
                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Use working directory as default.

            if (!*rundir_)
                realpath(rundir_, getcwd().c_str(), sizeof(rundir_));

            reconf();
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            verify(aug_setsrvlogger("aug"));

            auto_ptr<state> ptr(new state());
            setioeventmask(ptr->mplexer_, aug_eventin(), AUG_IOEVENTRD);
            state_ = ptr;
        }

        void
        do_run()
        {
            timeval tv;

            aug_info("running daemon process");

            state_->timer_.set(5000, *this);

            int ret(!0);
            while (0 < remain_) {

                if (state_->timers_.empty()) {

                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    foreachexpired(state_->timers_, 0 == ret, tv);
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                readevent();
            }
        }

        void
        do_term()
        {
            aug_info("terminating daemon process");
            state_.reset();
        }

        void
        do_callback(idref ref, unsigned& ms, aug_timers& timers)
        {
            aug_info("timer fired");
            --remain_;
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
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);
		service serv;

        program_ = argv[0];

        return main(argc, argv, serv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
