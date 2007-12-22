/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "augsys/defs.h"

AUG_RCSID("$Id$");

/**
 * @page daug
 *
 * TODO.
 */

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augaspp/engine.hpp"

#include "daug/exception.hpp"
#include "daug/module.hpp"
#include "daug/options.hpp"
#include "daug/session.hpp"
#include "daug/ssl.hpp"
#include "daug/utility.hpp"

#include <iomanip>
#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <sstream>

#include <time.h>

using namespace aub;
using namespace aug;
using namespace daug;
using namespace std;

namespace {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char DEFAULT_NAME[] = "default";

#if !defined(_WIN32)
    const char DEFAULT_MODULE[] = "./modskel.so";
    const char MODEXT[] = ".so";
#else // _WIN32
    const char DEFAULT_MODULE[] = "./modskel.dll";
    const char MODEXT[] = ".dll";
#endif // _WIN32

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    cstring logdir_ = "";
    options options_;
    bool daemon_(false);

    bool
    withso_(const string& s)
    {
        string::size_type n(s.size());
        return 3 < n
            && '.' == s[n - 3]
            && ('s' == s[n - 2] || 'S' == s[n - 2])
            && ('o' == s[n - 1] || 'O' == s[n - 1]);
    }

    bool
    withdll_(const string& s)
    {
        string::size_type n(s.size());
        return 4 < n
            && '.' == s[n - 4]
            && ('d' == s[n - 3] || 'D' == s[n - 3])
            && ('l' == s[n - 2] || 'L' == s[n - 2])
            && ('l' == s[n - 1] || 'L' == s[n - 1]);
    }

    bool
    withext_(const string& s)
    {
        return withso_(s) || withdll_(s);
    }

    void
    openlog_()
    {
        // The current date is appended to the log file name.

        tm tm;
        aug::gmtime(tm);
        stringstream ss;
        ss << "daug-" << setfill('0')
           << setw(4) << tm.tm_year + 1900
           << setw(2) << tm.tm_mon + 1
           << setw(2) << tm.tm_mday;
        openlog(makepath(logdir_, ss.str().c_str(), "log").c_str());
    }

    void
    reconf_()
    {
        if (*conffile_) {
            AUG_DEBUG2("reading config-file: path=[%s]", conffile_);
            options_.read(conffile_);
        }

        const char* value;

        // Always set log-level first so that any subsequent log statements
        // use the new level.

        if ((value = options_.get("loglevel", 0))) {
            unsigned level(strtoui(value, 10));
            AUG_DEBUG2("setting log level: level=[%d]", level);
            aug_setloglevel(level);
        }

        // Other config directories may be specified relative to the run
        // directory.

        aug::chdir(rundir_);
        if (daemon_) {

            aug::chdir(options_.get("logdir", "."));

            // Cache real path so that the log file can be re-opened without
            // having to change directories.

            realpath(logdir_, getcwd().c_str(), sizeof(logdir_));

            // Re-opening the log file facilitates rolling.

            openlog_();
        }

        aug_info("loglevel=[%d]", aug_loglevel());
        aug_info("rundir=[%s]", rundir_);
    }

    void
    reopencb_(objectref ob, int id, unsigned& ms)
    {
        AUG_DEBUG2("re-opening log file");
        openlog_();
    }

    class enginecb : public enginecb_base {
        void
        do_reconf()
        {
            if (*conffile_) {
                AUG_DEBUG2("reading config-file: path=[%s]", conffile_);
                options_.read(conffile_);
            }
            reconf_();
        }

    } enginecb_;

    typedef map<string, moduleptr> modules;

    struct state {

        modules modules_;
#if ENABLE_SSL
        sslctxs sslctxs_;
#endif // ENABLE_SSL
        timers timers_;
        engine engine_;

        explicit
        state(char* frobpass)
            : engine_(smartfd::retain(aug_eventrd()),
                      smartfd::retain(aug_eventwr()), timers_, enginecb_)
        {
#if ENABLE_SSL
            initssl();
            createsslctxs(sslctxs_, options_, frobpass);
#endif // ENABLE_SSL
        }
    };

    auto_ptr<state> state_;

    // Thread-safe.

    void
    writelog_(int level, const char* format, ...)
    {
        // Cannot throw.

        va_list args;
        va_start(args, format);
        aug_vwritelog(level, format, args);
        va_end(args);
    }

    // Thread-safe.

    void
    vwritelog_(int level, const char* format, va_list args)
    {
        // Cannot throw.

        aug_vwritelog(level, format, args);
    }

    // Thread-safe.

    const char*
    error_()
    {
        return aug_errdesc;
    }

    // Thread-safe.

    int
    reconfall_()
    {
        AUG_DEBUG2("reconfall()");
        try {
            state_->engine_.reconfall();
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    stopall_()
    {
        AUG_DEBUG2("stopall()");
        try {
            state_->engine_.stopall();
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    post_(const char* to, const char* type, aub_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_DEBUG2("post(): sname=[%s], to=[%s], type=[%s]", sname, to, type);
        try {
            state_->engine_.post(sname, to, type, ob);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    dispatch_(const char* to, const char* type, aub_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_DEBUG2("dispatch(): sname=[%s], to=[%s], type=[%s]",
                   sname, to, type);
        try {
            state_->engine_.dispatch(sname, to, type, ob);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const char*
    getenv_(const char* name, const char* def)
    {
        // Return absolute rundir.

        if (0 == strcmp(name, "rundir"))
            return rundir_;

        try {

            const char* value(options_.get(name, 0));

            /* Fallback to environment table. */

            if (!value)
                value = getenv(name);

            return value ? value : def;

        } AUG_SETERRINFOCATCH;
        return 0;
    }

    const aum_session*
    getsession_()
    {
        try {
            return getsession();
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    int
    shutdown_(aum_id cid, unsigned flags)
    {
        AUG_DEBUG2("shutdown(): id=[%d], flags=[%u]", cid, flags);
        try {
            state_->engine_.shutdown(cid, flags);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcpconnect_(const char* host, const char* port, void* user)
    {
        const char* sname = getsession()->name_;
        AUG_DEBUG2("tcpconnect(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {
            return (int)state_->engine_.tcpconnect(sname, host, port, user);

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcplisten_(const char* host, const char* port, void* user)
    {
        const char* sname = getsession()->name_;
        AUG_DEBUG2("tcplisten(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {

            // TODO: temporarily regain root privileges.

            return (int)state_->engine_.tcplisten(sname, host, port, user);

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    send_(aum_id cid, const void* buf, size_t len)
    {
        AUG_DEBUG2("send(): id=[%d]", cid);
        try {
            state_->engine_.send(cid, buf, len);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    sendv_(aum_id cid, aug_blob* blob)
    {
        AUG_DEBUG2("sendv(): id=[%d]", cid);
        try {
            state_->engine_.sendv(cid, blob);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setrwtimer_(aum_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("setrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            state_->engine_.setrwtimer(cid, ms, flags);
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resetrwtimer_(aum_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("resetrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            return state_->engine_.resetrwtimer(cid, ms, flags)
                ? 0 : AUM_NONE;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    cancelrwtimer_(aum_id cid, unsigned flags)
    {
        AUG_DEBUG2("cancelrwtimer(): id=[%d], flags=[%x]", cid, flags);
        try {
            return state_->engine_.cancelrwtimer(cid, flags)
                ? 0 : AUM_NONE;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    settimer_(unsigned ms, aub_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_DEBUG2("settimer(): sname=[%s], ms=[%u]", sname, ms);
        try {
            return (int)state_->engine_.settimer(sname, ms, ob);

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resettimer_(aum_id tid, unsigned ms)
    {
        AUG_DEBUG2("resettimer(): id=[%d], ms=[%u]", tid, ms);
        try {
            return state_->engine_.resettimer(tid, ms) ? 0 : AUM_NONE;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    canceltimer_(aum_id tid)
    {
        AUG_DEBUG2("canceltimer(): id=[%d]", tid);
        try {
            return state_->engine_.canceltimer(tid) ? 0 : AUM_NONE;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setsslclient_(aum_id cid, const char* ctx)
    {
        AUG_DEBUG2("setsslclient(): id=[%d], ctx=[%s]", cid, ctx);
#if ENABLE_SSL
        try {
            sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
            if (it == state_->sslctxs_.end())
                throw error(__FILE__, __LINE__, ESSLCTX,
                            "SSL context [%s] not initialised", ctx);

            state_->engine_.setsslclient(cid, *it->second);
            return 0;

        } AUG_SETERRINFOCATCH;
#else // !ENABLE_SSL
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setsslserver() not supported"));
#endif // !ENABLE_SSL
        return -1;
    }

    int
    setsslserver_(aum_id cid, const char* ctx)
    {
        AUG_DEBUG2("setsslserver(): id=[%d], ctx=[%s]", cid, ctx);
#if ENABLE_SSL
        try {
            sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
            if (it == state_->sslctxs_.end())
                throw error(__FILE__, __LINE__, ESSLCTX,
                            "SSL context [%s] not initialised", ctx);

            state_->engine_.setsslclient(cid, *it->second);
            return 0;

        } AUG_SETERRINFOCATCH;
#else // !ENABLE_SSL
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ESUPPORT,
                       AUG_MSG("aug_setsslserver() not supported"));
#endif // !ENABLE_SSL
        return -1;
    }

    const aum_host host_ = {
        writelog_,
        vwritelog_,
        error_,
        reconfall_,
        stopall_,
        post_,
        dispatch_,
        getenv_,
        getsession_,
        shutdown_,
        tcpconnect_,
        tcplisten_,
        send_,
        sendv_,
        setrwtimer_,
        resetrwtimer_,
        cancelrwtimer_,
        settimer_,
        resettimer_,
        canceltimer_,
        setsslclient_,
        setsslserver_
    };

    void
    teardown_(const aum_handle* sock)
    {
        aug_info("teardown defaulting to shutdown");
        shutdown_(sock->id_, 0);
    }

    void
    load_()
    {
        AUG_DEBUG2("loading sessions");

        // TODO: allow each session to specify a list of sessions on which it
        // depends.

        // Obtain list of sessions.

        const char* value(options_.get("sessions", 0));
        if (value) {

            // For each session...

            istringstream is(value);
            string name, value;
            while (is >> name) {

                // Obtain module associated with session.

                const string base(string("session.").append(name));
                value = options_.get(base + ".module");

                modules::iterator it(state_->modules_.find(value));
                if (it == state_->modules_.end()) {

                    // Load module.

                    string path(options_.get(string("module.").append(value)
                                             .append(".path")));
                    if (!withext_(path))
                        path += MODEXT;

                    aug_info("loading module: name=[%s], path=[%s]",
                             value.c_str(), path.c_str());
                    aug::chdir(rundir_);
                    moduleptr module(new daug::module(value, path.c_str(),
                                                      host_, teardown_));
                    it = state_->modules_
                        .insert(make_pair(value, module)).first;
                }

                aug_info("creating session: name=[%s]", name.c_str());
                state_->engine_.insert
                    (name, sessionptr(new daug::session(it->second,
                                                        name.c_str())),
                     options_.get(base + ".groups", 0));
            }

        } else {

            // No session list: assume reasonable defaults.

            aug_info("loading module: name=[%s]", DEFAULT_NAME);
            moduleptr module(new daug::module(DEFAULT_NAME, DEFAULT_MODULE,
                                              host_, teardown_));
            state_->modules_[DEFAULT_NAME] = module;

            aug_info("creating session: name=[%s]", DEFAULT_NAME);
            state_->engine_
                .insert(DEFAULT_NAME,
                        sessionptr(new daug::session(module, DEFAULT_NAME)),
                        0);
        }

        state_->engine_.cancelinactive();
    }

    class service {
        char frobpass_[AUG_MAXPASSWORD + 1];
    public:
        ~service() AUG_NOTHROW
        {
        }
        service()
        {
            frobpass_[0] = '\0';
        }

        const char*
        getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTEMAIL:
                return PACKAGE_BUGREPORT;
            case AUG_OPTLONGNAME:
                return "aug application server";
            case AUG_OPTPIDFILE:
                return options_.get("pidfile", "daug.pid");
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "daug";
            }
            return 0;
        }

        void
        readconf(const char* conffile, bool batch, bool daemon)
        {
            // The conffile is optional, if specified it will be an absolute
            // path.

            if (conffile) {

                AUG_DEBUG2("reading config-file: path=[%s]", conffile);
                options_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Once set, the run directory should not change.

            const char* rundir(options_.get("rundir", 0));
            realpath(rundir_, rundir ? rundir : getcwd().c_str(),
                     sizeof(rundir_));

            reconf_();

#if ENABLE_SSL
            // Password must be collected before process is detached from
            // controlling terminal.

            if (!batch && options_.get("ssl.contexts", 0)) {
                aug_getpass("Enter PEM pass phrase:", frobpass_,
                            sizeof(frobpass_));
                aug_memfrob(frobpass_, sizeof(frobpass_) - 1);
            }
#endif // ENABLE_SSL
        }

        void
        init()
        {
            AUG_DEBUG2("initialising daemon process");

            setsrvlogger("daug");

            auto_ptr<state> s(new state(frobpass_));

            // Assign state so that it is visible to callbacks during load_()
            // call.

            state_ = s;
            try {
                load_();
            } catch (...) {

                // Ownership back to local for cleanup.

                s = state_;
                s->engine_.clear();
                throw;
            }
        }

        void
        run()
        {
            if (!daemon_) {
                state_->engine_.run(true);  // Stop on error.
                return;
            }

            // Only set reopen timer when running as daemon.

            timer t(state_->timers_);
            t.set(60000, timercb<reopencb_>, null);
            state_->engine_.run(false); // Continue on error.
        }

        void
        term()
        {
            AUG_DEBUG2("terminating daemon process");

            // Clear services first.

            state_->engine_.clear();

            // Modules must be kept alive until remaining connections and
            // timers have been destroyed: a destroy_() function may depend on
            // a function implemented in a module.

            state_.reset();
        }
    };
}

int
main(int argc, char* argv[])
{
    try {

         // Initialise aug libraries.

        aug_errinfo errinfo;
        scoped_init init(errinfo);

        // Seed random number generator.

        timeval tv;
        aug::gettimeofday(tv);
        aug::srand(getpid() ^ tv.tv_sec ^ tv.tv_usec);

        try {
            service serv;
            program_ = argv[0];

            blocksignals();
            return main(argc, argv, serv);

        } AUG_PERRINFOCATCH;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
