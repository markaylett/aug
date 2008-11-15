/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "augctx/defs.h"

AUG_RCSID("$Id$");

/**
 * @page daug
 *
 * daug - a cross between the Dachshund and the Pug!  And, of course, the aug
 * application server.
 */

#include "augsrvpp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

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

using namespace aug;
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
        // The current date is appended to the log file name.  This provides a
        // convenient mechanism for rotating log files.

        tm tm;
        aug::gmtime(tm);
        stringstream ss;
        ss << "daug-" << setfill('0')
           << setw(4) << tm.tm_year + 1900
           << setw(2) << tm.tm_mon + 1
           << setw(2) << tm.tm_mday;

        // Re-direct standard handles.

        openlog(makepath(logdir_, ss.str().c_str(), "log").c_str());
    }

    void
    reconf_()
    {
        if (*conffile_) {
            AUG_CTXDEBUG2(aug_tlx, "reading config-file: path=[%s]",
                          conffile_);
            options_.read(conffile_);
        }

        const char* value;

        // Always set log-level first so that any subsequent log statements
        // use the new level.

        if ((value = options_.get("loglevel", 0))) {
            unsigned level(strtoui(value, 10));
            AUG_CTXDEBUG2(aug_tlx, "setting log level: level=[%d]", level);
            aug_setloglevel(aug_tlx, level);
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

        aug_ctxinfo(aug_tlx, "loglevel=[%d]", aug_getloglevel(aug_tlx));
        aug_ctxinfo(aug_tlx, "rundir=[%s]", rundir_);
    }

    void
    reopencb_(objectref ob, idref id, unsigned& ms)
    {
        // Called by timer when running as daemon.

        AUG_CTXDEBUG2(aug_tlx, "re-opening log file");
        openlog_();
    }

    class enginecb : public enginecb_base {
        void
        do_reconf()
        {
            if (*conffile_) {
                AUG_CTXDEBUG2(aug_tlx, "reading config-file: path=[%s]",
                              conffile_);
                options_.read(conffile_);
            }
            reconf_();
        }

    } enginecb_;

    typedef map<string, moduleptr> modules;

    struct state : mpool_ops {

        modules modules_;
#if ENABLE_SSL
        sslctxs sslctxs_;
#endif // ENABLE_SSL
        timers timers_;
        engine engine_;

        explicit
        state(char* frobpass)
            : timers_(getmpool(aug_tlx)),
              engine_(aug_eventrd(), aug_eventwr(), timers_, enginecb_)
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
        aug_vctxlog(aug_tlx, level, format, args);
        va_end(args);
    }

    // Thread-safe.

    void
    vwritelog_(int level, const char* format, va_list args)
    {
        // Cannot throw.

        aug_vctxlog(aug_tlx, level, format, args);
    }

    // Thread-safe.

    const char*
    error_()
    {
        return aug_tlerr->desc_;
    }

    // Thread-safe.

    mod_result
    reconfall_()
    {
        AUG_CTXDEBUG2(aug_tlx, "reconfall()");
        try {
            state_->engine_.reconfall();
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    // Thread-safe.

    mod_result
    stopall_()
    {
        AUG_CTXDEBUG2(aug_tlx, "stopall()");
        try {
            state_->engine_.stopall();
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    // Thread-safe.

    mod_result
    post_(const char* to, const char* type, aug_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_CTXDEBUG2(aug_tlx, "post(): sname=[%s], to=[%s], type=[%s]",
                      sname, to, type);
        try {
            state_->engine_.post(sname, to, type, ob);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    dispatch_(const char* to, const char* type, aug_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_CTXDEBUG2(aug_tlx, "dispatch(): sname=[%s], to=[%s], type=[%s]",
                      sname, to, type);
        try {
            state_->engine_.dispatch(sname, to, type, ob);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
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

    const mod_session*
    getsession_()
    {
        try {
            return getsession();
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    mod_result
    shutdown_(mod_id cid, unsigned flags)
    {
        AUG_CTXDEBUG2(aug_tlx, "shutdown(): id=[%u], flags=[%u]", cid, flags);
        try {
            state_->engine_.shutdown(cid, flags);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    int
    tcpconnect_(const char* host, const char* port, const char* ctx,
                void* user)
    {
        const char* sname = getsession()->name_;
        AUG_CTXDEBUG2(aug_tlx,
                      "tcpconnect(): sname=[%s], host=[%s], port=[%s]",
                      sname, host, port);
        try {
            sslctx* ptr(0);
#if ENABLE_SSL
            if (ctx) {
                sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
                if (it == state_->sslctxs_.end())
                    throw daug_error(__FILE__, __LINE__, ESSLCTX,
                                     "SSL context [%s] not initialised", ctx);
                ptr = it->second.get();
            }
#endif // ENABLE_SSL
            return (int)state_->engine_
                .tcpconnect(sname, host, port, ptr, user);

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    int
    tcplisten_(const char* host, const char* port, const char* ctx,
               void* user)
    {
        const char* sname = getsession()->name_;
        AUG_CTXDEBUG2(aug_tlx,
                      "tcplisten(): sname=[%s], host=[%s], port=[%s]",
                      sname, host, port);
        try {
            sslctx* ptr(0);
#if ENABLE_SSL
            if (ctx) {
                sslctxs::const_iterator it(state_->sslctxs_.find(ctx));
                if (it == state_->sslctxs_.end())
                    throw daug_error(__FILE__, __LINE__, ESSLCTX,
                                     "SSL context [%s] not initialised", ctx);
                ptr = it->second.get();
            }
#endif // ENABLE_SSL

            // TODO: temporarily regain root privileges.

            return (int)state_->engine_
                .tcplisten(sname, host, port, ptr, user);

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    send_(mod_id cid, const void* buf, size_t len)
    {
        AUG_CTXDEBUG2(aug_tlx, "send(): id=[%u]", cid);
        try {
            state_->engine_.send(cid, buf, len);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    sendv_(mod_id cid, aug_blob* blob)
    {
        AUG_CTXDEBUG2(aug_tlx, "sendv(): id=[%u]", cid);
        try {
            state_->engine_.sendv(cid, blob);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    setrwtimer_(mod_id cid, unsigned ms, unsigned flags)
    {
        AUG_CTXDEBUG2(aug_tlx, "setrwtimer(): id=[%u], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            state_->engine_.setrwtimer(cid, ms, flags);
            return MOD_SUCCESS;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    resetrwtimer_(mod_id cid, unsigned ms, unsigned flags)
    {
        AUG_CTXDEBUG2(aug_tlx, "resetrwtimer(): id=[%u], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            return state_->engine_.resetrwtimer(cid, ms, flags)
                ? MOD_SUCCESS : MOD_FAILNONE;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    cancelrwtimer_(mod_id cid, unsigned flags)
    {
        AUG_CTXDEBUG2(aug_tlx, "cancelrwtimer(): id=[%u], flags=[%x]",
                      cid, flags);
        try {
            return state_->engine_.cancelrwtimer(cid, flags)
                ? MOD_SUCCESS : MOD_FAILNONE;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    int
    settimer_(unsigned ms, aug_object* ob)
    {
        const char* sname = getsession()->name_;
        AUG_CTXDEBUG2(aug_tlx, "settimer(): sname=[%s], ms=[%u]", sname, ms);
        try {
            return (int)state_->engine_.settimer(sname, ms, ob);

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    resettimer_(mod_id tid, unsigned ms)
    {
        AUG_CTXDEBUG2(aug_tlx, "resettimer(): id=[%u], ms=[%u]", tid, ms);
        try {
            return state_->engine_.resettimer(tid, ms)
                ? MOD_SUCCESS : MOD_FAILNONE;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    mod_result
    canceltimer_(mod_id tid)
    {
        AUG_CTXDEBUG2(aug_tlx, "canceltimer(): id=[%u]", tid);
        try {
            return state_->engine_.canceltimer(tid)
                ? MOD_SUCCESS : MOD_FAILNONE;

        } AUG_SETERRINFOCATCH;
        return MOD_FAILERROR;
    }

    const mod_host host_ = {
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
        canceltimer_
    };

    void
    teardown_(const mod_handle* sock)
    {
        // Teardown is a protocol-level shutdown.

        aug_ctxinfo(aug_tlx, "teardown defaulting to shutdown");
        shutdown_(sock->id_, 0);
    }

    void
    load_()
    {
        // Called from init().

        AUG_CTXDEBUG2(aug_tlx, "loading sessions");

        // TODO: allow each session to specify a list of sessions on which it
        // depends.

        // Obtain list of sessions from config.  Default is null.

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

                    // Module does not yet exist, so load now.

                    string path(options_.get(string("module.").append(value)
                                             .append(".path")));

                    // Allow modules to be specified with or without a file
                    // extension.  This aids portability.

                    if (!withext_(path))
                        path += MODEXT;

                    aug_ctxinfo(aug_tlx,
                                "loading module: name=[%s], path=[%s]",
                                value.c_str(), path.c_str());

                    // Module can assume to be in run directory during
                    // intialisation.

                    aug::chdir(rundir_);
                    moduleptr module(new daug::module(value, path.c_str(),
                                                      host_, teardown_));
                    it = state_->modules_
                        .insert(make_pair(value, module)).first;
                }

                aug_ctxinfo(aug_tlx,
                            "creating session: name=[%s]", name.c_str());
                state_->engine_.insert
                    (name, sessionptr(new daug::session(it->second,
                                                        name.c_str())),
                     options_.get(base + ".groups", 0));
            }

        } else {

            // No session list: user reasonable defaults.

            aug_ctxinfo(aug_tlx, "loading module: name=[%s]", DEFAULT_NAME);
            moduleptr module(new daug::module(DEFAULT_NAME, DEFAULT_MODULE,
                                              host_, teardown_));
            state_->modules_[DEFAULT_NAME] = module;

            aug_ctxinfo(aug_tlx, "creating session: name=[%s]", DEFAULT_NAME);
            state_->engine_
                .insert(DEFAULT_NAME,
                        sessionptr(new daug::session(module, DEFAULT_NAME)),
                        0);
        }

        // A session is active once start() has returned true.

        // Sessions may create timers during start().  If start() subsequently
        // fails, these timers will need to be cancelled.

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

        aug_result
        readconf(const char* conffile, bool batch, bool daemon)
        {
            // The conffile is optional, if specified it will be an absolute
            // path.

            if (conffile) {

                AUG_CTXDEBUG2(aug_tlx, "reading config-file: path=[%s]",
                              conffile);
                options_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            // Remember if daemonising or not.

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

            return AUG_SUCCESS;
        }

        aug_result
        init()
        {
            AUG_CTXDEBUG2(aug_tlx, "initialising daemon process");

            setsrvlogger("daug");

            auto_ptr<state> s(new state(frobpass_));

            // Assign state so that it is visible to callbacks during load_().

            state_ = s;
            try {

                // Load modules are start sessions.

                load_();

            } catch (...) {

                // Ownership back to local for cleanup.

                s = state_;
                s->engine_.clear();
                throw;
            }

            return AUG_SUCCESS;
        }

        aug_result
        run()
        {
            if (daemon_) {

                // Only set re-open timer when running as daemon.

                timer t(state_->timers_);
                t.set(60000, timercb<reopencb_>, null);

                state_->engine_.run(false); // Not stop on error.

            } else
                state_->engine_.run(true);  // Stop on error.

            return AUG_SUCCESS;
        }

        void
        term()
        {
            AUG_CTXDEBUG2(aug_tlx, "terminating daemon process");

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

        scoped_init init(basictlx);

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
