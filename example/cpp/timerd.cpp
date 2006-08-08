/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id:$";

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
    char rundir_[AUG_PATH_MAX + 1];
    char pidfile_[AUG_PATH_MAX + 1] = "timerd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "timerd.log";

    class confcb : public confcb_base {
        void
        do_callback(const char* name, const char* value)
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

                if (!aug_realpath(rundir_, value, sizeof(rundir_))) {
                    aug_setposixerrinfo(__FILE__, __LINE__, errno);
                    throwerrinfo("aug_realpath() failed");
                }

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
            struct aug_event event;

            AUG_DEBUG("checking event pipe '%d'", fd);

            if (!ioevents(state_->mplexer_, fd))
                return;

            AUG_DEBUG("reading event");

            switch (aug::readevent(aug_eventin(), event).type_) {
            case AUG_EVENTRECONF:
                aug_info("received AUG_EVENTRECONF");
                reconfig();
                break;
            case AUG_EVENTSTATUS:
                aug_info("received AUG_EVENTSTATUS");
                break;
            case AUG_EVENTSTOP:
                aug_info("received AUG_EVENTSTOP");
                remain_ = 0;
                break;
            }
        }

        void
        reconfig()
        {
            if (*conffile_) {

                aug_info("reading: %s", conffile_);

                test::confcb cb;
                readconf(conffile_, cb);
            }

            if (-1 == chdir(rundir_)) {
                aug_setposixerrinfo(__FILE__, __LINE__, errno);
                throwerrinfo("chdir() failed");
            }

            if (daemon_ && -1 == aug_openlog(logfile_)) {
                aug_setposixerrinfo(__FILE__, __LINE__, errno);
                throwerrinfo("aug_openlog() failed");
            }

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
                                          sizeof(conffile_))) {
                aug_setposixerrinfo(__FILE__, __LINE__, errno);
                throwerrinfo("aug_realpath() failed");
            }

            daemon_ = daemon;
            reconfig();
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            if (-1 == aug_setsrvlogger("aug")) {
                aug_setposixerrinfo(__FILE__, __LINE__, errno);
                throwerrinfo("aug_setsrvlogger() failed");
            }

            auto_ptr<state> ptr(new state());
            setioeventmask(ptr->mplexer_, aug_eventin(), AUG_IOEVENTRD);
            state_ = ptr;
        }

        void
        do_run()
        {
            struct timeval tv;

            aug_info("running daemon process");

            state_->timer_.set(5000, *this);

            int ret(!0);
            while (0 < remain_) {

                if (state_->timers_.empty()) {

                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    processtimers(state_->timers_, 0 == ret, tv);
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                readevent();
            }

            aug_info("stopping daemon process");
        }

        void
        do_callback(idref ref, unsigned int& ms, struct aug_timers& timers)
        {
            aug_info("timer fired");
            --remain_;
        }

    public:
        ~service() NOTHROW
        {
        }

        service()
            : daemon_(false),
              remain_(5)
        {
        }
    };

    string
    getcwd()
    {
        char buf[AUG_PATH_MAX + 1];
        if (!::getcwd(buf, sizeof(buf))) {
            aug_setposixerrinfo(__FILE__, __LINE__, errno);
            throwerrinfo("getcwd() failed");
        }

        return buf;
    }
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        struct aug_errinfo errinfo;
        scoped_init init(errinfo);
		service serv;

        program_ = argv[0];

        if (!getcwd(rundir_, sizeof(rundir_))) {
            aug_setposixerrinfo(__FILE__, __LINE__, errno);
            throwerrinfo("getcwd() failed");
        }

        main(serv, argc, argv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
