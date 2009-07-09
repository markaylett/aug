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
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>
#include <memory>

#include <time.h>

using namespace aug;
using namespace std;

namespace test {

    const char* program_;

    char conffile_[AUG_PATH_MAX + 1] = "";
    bool daemon_(false);

    char rundir_[AUG_PATH_MAX + 1] = "";
    char pidfile_[AUG_PATH_MAX + 1] = "timerd.pid";
    char logfile_[AUG_PATH_MAX + 1] = "timerd.log";

    void
    reconf_()
    {
        char path[AUG_PATH_MAX + 1];
        if (daemon_)
            openlog(abspath(path, rundir_, logfile_, sizeof(path)));

        aug_ctxinfo(aug_tlx, "run directory: %s", rundir_);
        aug_ctxinfo(aug_tlx, "pid file: %s", pidfile_);
        aug_ctxinfo(aug_tlx, "log file: %s", logfile_);
        aug_ctxinfo(aug_tlx, "log level: %d", aug_loglevel());
    }

    aug_result
    confcb_(const char* name, const char* value, void* arg)
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

    class impl : public task_base<impl>, public mpool_ops {

        struct state : mpool_ops {
            muxer muxer_;
            timers timers_;
            timer timer_;
        public:
            state()
                : muxer_(getmpool(aug_tlx)),
                  timers_(getmpool(aug_tlx), getclock(aug_tlx)),
                  timer_(timers_, null)
            {
            }
        };

        int remain_;
        auto_ptr<state> state_;

        void
        readevent()
        {
            mdref md(eventsmd());

            AUG_CTXDEBUG2(aug_tlx, "checking event pipe [%d]", md.get());

            if (!getmdevents(state_->muxer_, md))
                return;

            AUG_CTXDEBUG2(aug_tlx, "reading event");

            // Sticky events not required for fixed length blocking read.

            pair<int, objectptr> event(aug::readevent(aug_events()));

            switch (event.first) {
            case AUG_EVENTRECONF:
                aug_ctxinfo(aug_tlx, "received AUG_EVENTRECONF");
                if (*conffile_) {
                    aug_ctxinfo(aug_tlx, "reading: %s", conffile_);
                    aug::readconf(conffile_, aug::confcb<confcb_>, null);
                }
                reconf_();
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
        run()
        {
            aug_timeval tv;

            aug_ctxinfo(aug_tlx, "running daemon process");

            state_->timer_.set(5000, *this);

            unsigned ready(!0);
            while (0 < remain_) {

                processexpired(state_->timers_, 0 == ready, tv);

                try {

                    scoped_sigunblock unblock;

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
        }

    public:
        ~impl() AUG_NOTHROW
        {
            aug_ctxinfo(aug_tlx, "terminating daemon process");
            state_.reset();
        }

        impl()
            : remain_(5)
        {
            aug_ctxinfo(aug_tlx, "initialising daemon process");

            auto_ptr<state> ptr(new (tlx) state());
            setmdeventmask(ptr->muxer_, eventsmd(), AUG_MDEVENTRDEX);
            state_ = ptr;
        }

        aug_result
        runtask_() AUG_NOTHROW
        {
            try {
                run();
                return AUG_SUCCESS;
            } AUG_SETERRINFOCATCH;
            return AUG_FAILERROR;
        }

        void
        timercb(aug_id id, unsigned& ms)
        {
            aug_ctxinfo(aug_tlx, "timer fired");
            --remain_;
        }
    };

    const char*
    getopt_(int opt) AUG_NOTHROW
    {
        switch (opt) {
        case AUG_OPTEMAIL:
            return "Mark Aylett <mark.aylett@gmail.com>";
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
    readconf_(const char* conffile, aug_bool batch,
              aug_bool daemon) AUG_NOTHROW
    {
        try {

            if (conffile) {
                aug_ctxinfo(aug_tlx, "reading: %s", conffile);
                aug::readconf(conffile, aug::confcb<confcb_>, null);
                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon ? true : false;;

            // Use working directory as default.

            if (!*rundir_)
                realpath(rundir_, getcwd().c_str(), sizeof(rundir_));

            reconf_();
            return AUG_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return AUG_FAILERROR;
    }

    aug_task*
    create_() AUG_NOTHROW
    {
        try {
            setservlogger("aug");
            return retget(impl::attach(new (tlx) impl()));
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    const aug_serv serv_ = {
        getopt_,
        readconf_,
        create_
    };
}

int
main(int argc, char* argv[])
{
    using namespace test;

    try {

        autotlx();

        program_ = argv[0];

        return main(argc, argv, serv_);

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }

    return 1; // aug_main() does not return.
}
