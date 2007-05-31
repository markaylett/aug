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

    void
    confcb(void* arg, const char* name, const char* value)
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

    class server {

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
            int fd(aug_eventrd());
            aug_event event;

            AUG_DEBUG2("checking event pipe '%d'", fd);

            if (!fdevents(state_->mplexer_, fd))
                return;

            AUG_DEBUG2("reading event");

            switch (aug::readevent(aug_eventrd(), event).type_) {
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

    public:
        ~server() AUG_NOTHROW
        {
        }

        server()
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

        void
        readconf(const char* conffile, bool prompt, bool daemon)
        {
            if (conffile) {
                aug_info("reading: %s", conffile);
                aug::readconf(conffile, aug::confcb<confcb>, null);
                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Use working directory as default.

            if (!*rundir_)
                realpath(rundir_, getcwd().c_str(), sizeof(rundir_));

            reconf();
        }

        void
        init()
        {
            aug_info("initialising daemon process");

            verify(aug_setsrvlogger("aug"));

            auto_ptr<state> ptr(new state());
            setfdeventmask(ptr->mplexer_, aug_eventrd(), AUG_FDEVENTRD);
            state_ = ptr;
        }

        void
        run()
        {
            timeval tv;

            aug_info("running daemon process");

            state_->timer_.set(5000, *this);

            int ret(!0);
            while (0 < remain_) {

                if (state_->timers_.empty()) {

                    while (AUG_RETINTR == (ret = waitfdevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    foreachexpired(state_->timers_, 0 == ret, tv);
                    while (AUG_RETINTR == (ret = waitfdevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                readevent();
            }
        }

        void
        term()
        {
            aug_info("terminating daemon process");
            state_.reset();
        }

        void
        timercb(int id, unsigned& ms)
        {
            aug_info("timer fired");
            --remain_;
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
		server serv;

        program_ = argv[0];

        return main(argc, argv, serv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
