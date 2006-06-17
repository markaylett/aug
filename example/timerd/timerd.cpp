/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id:$";

#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include <memory>
#include <time.h>

#if defined(_MSC_VER)
# pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

using namespace aug;
using namespace std;

namespace test {

    const char* program_;

    char conffile_[AUG_PATH_MAX + 1] = "";
    char rundir_[AUG_PATH_MAX + 1];
    char pidfile_[AUG_PATH_MAX + 1] = "timerd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "timerd.log";

    class setopt : public setopt_base {
        void
        do_setopt(const char* name, const char* value)
        {
            if (0 == aug_strcasecmp(name, "loglevel")) {

                unsigned int level(strtoui(value, 10));
                aug_info("setting log level: %d", level);
                aug_setloglevel(level);

            } else if (0 == aug_strcasecmp(name, "logfile")) {

                aug_strlcpy(logfile_, value, sizeof(logfile_));

            } else if (0 == aug_strcasecmp(name, "pidfile")) {

                aug_strlcpy(pidfile_, value, sizeof(pidfile_));

            } else if (0 == aug_strcasecmp(name, "rundir")) {

                if (!aug_realpath(rundir_, value, sizeof(rundir_)))
                    error("aug_realpath() failed");

            } else {

                throw runtime_error("option not supported");
            }
        }
    };

    class service : public service_base, public expire_base {

        struct state {
            mplexer mplexer_;
            timers timers_;
            timer timer_;
        public:
            explicit
            state(expire_base& expire)
                : timer_(timers_, expire)
            {
            }
        };

        bool daemon_;
        int remain_;
        auto_ptr<state> state_;

        void
        checkpipe()
        {
            int fd(aug_signalin());

            AUG_DEBUG("checking signal pipe '%d'", fd);

            if (!events(state_->mplexer_, fd))
                return;

            AUG_DEBUG("reading signal action");

            switch (readsignal(aug_signalin())) {
            case AUG_SIGRECONF:
                aug_info("received AUG_SIGRECONF");
                reconfig();
                break;
            case AUG_SIGSTATUS:
                aug_info("received AUG_SIGSTATUS");
                break;
            case AUG_SIGSTOP:
                aug_info("received AUG_SIGSTOP");
                remain_ = 0;
                break;
            }
        }

        void
        reconfig()
        {
            if (*conffile_) {

                aug_info("reading: %s", conffile_);

                test::setopt action;
                readconf(conffile_, action);
            }

            if (-1 == chdir(rundir_))
                error("chdir() failed");

            if (daemon_ && -1 == aug_openlog(logfile_))
                error("aug_openlog() failed");

            aug_info("run directory: %s", rundir_);
            aug_info("pid file: %s", pidfile_);
            aug_info("log file: %s", logfile_);
            aug_info("log level: %d", aug_loglevel());
        }

        const char*
        do_getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTADMIN:
                return "Mark Aylett <mark@emantic.co.uk>";
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
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
        do_config(const char* conffile, bool daemon)
        {
            if (conffile && !aug_realpath(conffile_, conffile,
                                          sizeof(conffile_)))
                error("aug_realpath() failed");

            daemon_ = daemon;
            reconfig();
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            if (-1 == aug_setsrvlogger("timerd"))
                error("aug_setsrvlogger() failed");

            auto_ptr<state> ptr(new state(*this));
            seteventmask(ptr->mplexer_, aug_signalin(), AUG_EVENTRD);
            state_ = ptr;
        }

        void
        do_run()
        {
            struct timeval tv;

            aug_info("running daemon process");

            state_->timer_.reset(5000);

            int ret(!0);
            while (0 < remain_) {

                if (state_->timers_.empty()) {

                    while (AUG_RETINTR == (ret = waitevents(state_
                                                            ->mplexer_)))
                        ;

                } else {

                    processtimers(state_->timers_, 0 == ret, tv);
                    while (AUG_RETINTR == (ret = waitevents(state_
                                                            ->mplexer_, tv)))
                        ;
                }

                checkpipe();
            }

            aug_info("stopping daemon process");
        }

        void
        do_expire(int id)
        {
            aug_info("timer fired");
            --remain_;
            state_->timer_.reset(5000);
        }

    public:
        ~service() NOTHROW
        {
        }

        service()
            : daemon_(false),
              remain_(10)
        {
        }
    } service_;

    string
    getcwd()
    {
        char buf[AUG_PATH_MAX + 1];
        if (!::getcwd(buf, sizeof(buf)))
            error("getcwd() failed");

        return buf;
    }
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        struct aug_errinfo errinfo;
        initialiser init(errinfo);

        program_ = argv[0];

        if (!getcwd(rundir_, sizeof(rundir_)))
            error("getcwd() failed");

        main(service_, argc, argv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
