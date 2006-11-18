/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augas/buffer.hpp"
#include "augas/exception.hpp"
#include "augas/manager.hpp"
#include "augas/module.hpp"
#include "augas/options.hpp"
#include "augas/utility.hpp"

#if !defined(_WIN32)
# include "augconfig.h"
#else // _WIN32
# define PACKAGE_BUGREPORT "mark@emantic.co.uk"
#endif // _WIN32

#include <cassert>
#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <vector>

#include <time.h>

using namespace aug;
using namespace std;

namespace augas {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    options options_;
    bool daemon_(false);
    bool stopping_(false);

    void
    reconf()
    {
        const char* value;
        if ((value = options_.get("loglevel", 0))) {
            unsigned level(strtoui(value, 10));
            aug_info("setting log level: %d", level);
            aug_setloglevel(level);
        }

        // Other directories may be specified relative to the run directory.

        aug::chdir(rundir_);
        if (daemon_) {

            // Re-opening the log file facilitates rolling.

            openlog(options_.get("logfile", "augasd.log"));
        }

        aug_info("log level: %d", aug_loglevel());
        aug_info("run directory: %s", rundir_);
    }

    typedef map<int, sessptr> pending;
    typedef map<unsigned, pair<sessptr, void*> > events;

    struct state {

        conncb_base& cb_;
        mutex mutex_;
        aug::conns conns_;
        timers timers_;
        mplexer mplexer_;
        manager manager_;
        pending pending_;
        events events_;
        string lastError_;

        explicit
        state(conncb_base& cb)
            : cb_(cb)
        {
            aug_info("adding event pipe");
            insertconn(conns_, aug_eventin(), cb);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);
        }
    };

    auto_ptr<state> state_;
    const aug_driver* base_(0);

    int
    extclose(int fd)
    {
        aug_info("clearing io event mask '%d'", fd);
        aug_setioeventmask(state_->mplexer_, fd, 0);
        return base_->close_(fd);
    }

    void
    setextdriver(fdref ref, conncb_base& cb, unsigned short mask)
    {
        // Override close function.

        static aug_driver extended = { extclose, 0, 0, 0, 0, 0 };
        if (!base_) {
            base_ = &getdriver(ref);
            extdriver(extended, *base_);
        }

        insertconn(state_->conns_, ref, cb);
        try {
            setioeventmask(state_->mplexer_, ref, mask);
            setdriver(ref, extended);
        } catch (...) {
            removeconn(state_->conns_, ref);
            throw;
        }
    }

    void
    timercb_(int id, const aug_var* arg, unsigned* ms, aug_timers* timers)
    {
        aug_info("custom timer expiry");

        pending::iterator it(state_->pending_.find(id));
        sessptr sess(it->second);
        sess->expire(id, aug_getvarp(arg), *ms);
        if (0 == *ms) // What if canceltimer is used?
            state_->pending_.erase(it);
    }

    const char*
    error_()
    {
        return state_->lastError_.c_str();
    }

    void
    writelog_(int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        aug_vwritelog(level, format, args);
        va_end(args);
    }

    void
    vwritelog_(int level, const char* format, va_list args)
    {
        aug_vwritelog(level, format, args);
    }

    const char*
    getenv_(const char* sname, const char* name)
    {
        return options_.get(string(sname).append(".").append(name)
                            .c_str(), 0);
    }

    int
    tcpconnect_(const char* sname, const char* host, const char* serv,
                void* user)
    {
        sessptr sess(state_->manager_.getsess(sname));

        endpoint ep(null);
        smartfd sfd(tcpconnect(host, serv, ep));

        setnodelay(sfd, true);
        setnonblock(sfd, true);
        setextdriver(sfd, state_->cb_, AUG_IOEVENTRD);

        unsigned id(aug_nextid());
        state_->manager_.insert(connptr(new conn(sess, sfd, id, ep,
                                                 state_->timers_)));

        inetaddr addr(null);
        aug_info("connected to host '%s', port '%d'",
                 inetntop(getinetaddr(ep, addr)).c_str(),
                 static_cast<int>(ntohs(port(ep))));
        return 0;
    }
    int
    tcplisten_(const char* sname, const char* host, const char* serv,
               void* user)
    {
        endpoint ep(null);
        smartfd sfd(tcplisten(host, serv, ep));
        setextdriver(sfd, state_->cb_, AUG_IOEVENTRD);

        sessptr sess(state_->manager_.getsess(sname));
        state_->manager_.insert(sess, sfd);

        inetaddr addr(null);
        aug_info("listening on interface '%s', port '%d'",
                 inetntop(getinetaddr(ep, addr)).c_str(),
                 static_cast<int>(ntohs(port(ep))));
        return 0;
    }
    int
    post_(const char* sname, int type, void* user)
    {
        unsigned id(aug_nextid());
        {
            scoped_lock l(state_->mutex_);
            sessptr sess(state_->manager_.getsess(sname));
            state_->events_[id] = make_pair(sess, user);
        }

        aug_event e;
        e.type_ = type + AUG_EVENTUSER;
        aug_setvarl(&e.arg_, id);
        writeevent(aug_eventout(), e);
        return 0;
    }

    int
    settimer_(const char* sname, int id, unsigned ms, void* arg)
    {
        var v(arg);
        id = aug_settimer(cptr(state_->timers_), id, ms, timercb_, cptr(v));
        if (0 < id) {
            scoped_lock l(state_->mutex_);
            state_->pending_[id] = state_->manager_.getsess(sname);
        }
        return id;
    }

    int
    resettimer_(const char* sname, int tid, unsigned ms)
    {
        int ret(aug_resettimer(cptr(state_->timers_), tid, ms));
        if (ret < 0)
            state_->pending_.erase(tid);
        return ret;
    }

    int
    canceltimer_(const char* sname, int tid)
    {
        int ret(aug_canceltimer(cptr(state_->timers_), tid));
        state_->pending_.erase(tid);
        return ret;
    }

    int
    shutdown_(augas_id cid)
    {
        connptr conn(state_->manager_.getbyid(cid));
        conn->shutdown();
        return 0;
    }

    int
    send_(const char* sname, augas_id cid, const char* buf, size_t size,
          unsigned flags)
    {
        switch (flags) {
        case AUGAS_SNDALL:
            if (!state_->manager_.sendall(state_->mplexer_, cid, sname, buf,
                                          size)) {
                state_->lastError_ = "connection has been shutdown";
                return -1;
            }
            return 0;
        case AUGAS_SNDSELF:
            if (!state_->manager_.sendself(state_->mplexer_, cid, buf,
                                           size)) {
                state_->lastError_ = "connection has been shutdown";
                return -1;
            }
            return 0;
        case AUGAS_SNDOTHER:
            state_->manager_.sendother(state_->mplexer_, cid, sname, buf,
                                       size);
            return 0;
        }

        state_->lastError_ = "invalid flags";
        return -1;
    }

    int
    setrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        connptr conn(state_->manager_.getbyid(cid));
        conn->setrwtimer(ms, flags);
        return 0;
    }

    int
    resetrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        connptr conn(state_->manager_.getbyid(cid));
        conn->resetrwtimer(ms, flags);
        return 0;
    }

    int
    cancelrwtimer_(augas_id cid, unsigned flags)
    {
        connptr conn(state_->manager_.getbyid(cid));
        conn->cancelrwtimer(flags);
        return 0;
    }

    const struct augas_host host_ = {
        error_,
        writelog_,
        vwritelog_,
        getenv_,
        tcpconnect_,
        tcplisten_,
        post_,
        settimer_,
        resettimer_,
        canceltimer_,
        shutdown_,
        send_,
        setrwtimer_,
        resetrwtimer_,
        cancelrwtimer_
    };

    void
    load(conncb_base& cb)
    {
        aug_info("loading sessions");
        state_->manager_.load(options_, host_);

        // TODO: check isopen().
    }

    bool
    accept(fdref ref, const sessptr& sess, conncb_base& conncb)
    {
        aug_endpoint ep;

        AUG_DEBUG("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(ref, ep);

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
        setextdriver(sfd, conncb, AUG_IOEVENTRD);

        unsigned id(aug_nextid());
        state_->manager_.insert
            (connptr(new conn(sess, sfd, id, ep, state_->timers_)));

        return true;
    }

    bool
    readevent(conncb_base& cb)
    {
        aug_event event;
        AUG_DEBUG("reading event");

        switch (aug::readevent(aug_eventin(), event).type_) {
        case AUG_EVENTRECONF:
            aug_info("received AUG_EVENTRECONF");
            if (*conffile_) {
                aug_info("reading: %s", conffile_);
                options_.read(conffile_);
            }
            load(cb);
            reconf();
            {
                scoped_lock l(state_->mutex_);
                state_->manager_.reconf();
            }
            break;
        case AUG_EVENTSTATUS:
            aug_info("received AUG_EVENTSTATUS");
            break;
        case AUG_EVENTSTOP:
            aug_info("received AUG_EVENTSTOP");
            stopping_ = true;
            state_->manager_.teardown();
            break;
        default:
            assert(AUG_VTLONG == event.arg_.type_);
            {
                unsigned id(aug_getvarl(&event.arg_));

                scoped_lock l(state_->mutex_);
                events::iterator it(state_->events_.find(id));
                it->second.first->event(event.type_ - AUG_EVENTUSER,
                                        it->second.second);
                state_->events_.erase(it);
            }
        }
        return true;
    }

    class service : public conncb_base, public service_base {

        bool
        do_callback(int fd, aug_conns& conns)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            if (fd == aug_eventin())
                return readevent(*this);

            sessptr sess(state_->manager_.islistener(fd));
            if (sess != null)
                return accept(fd, sess, *this);

            connptr conn(state_->manager_.getbyfd(fd));
            if (!conn->process(state_->mplexer_)) {
                state_->manager_.erase(conn);
                return false;
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
                return PACKAGE_BUGREPORT;
            case AUG_OPTLONGNAME:
                return "aug application server";
            case AUG_OPTPIDFILE:
                return options_.get("pidfile", "augasd.pid");
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "augasd";
            }
            return 0;
        }

        void
        do_readconf(const char* conffile, bool daemon)
        {
            // The conffile is optional, if specified it will be an absolute
            // path.

            if (conffile) {

                aug_info("reading: %s", conffile);
                options_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Once set, the run directory should not change.

            const char* rundir(options_.get("rundir", 0));
            realpath(rundir_, rundir ? rundir : getcwd().c_str(),
                     sizeof(rundir_));

            reconf();
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            setsrvlogger("augasd");

            auto_ptr<state> s(new state(*this));
            state_ = s;
            try {
                load(*this);
            } catch (...) {
                s = state_;
                throw;
            }
        }

        void
        do_run()
        {
            struct timeval tv;

            aug_info("running daemon process");

            int ret(!0);
            while (!stopping_ || !state_->manager_.isconnected()) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    processtimers(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                processconns(state_->conns_);
            }
        }

        void
        do_term()
        {
            aug_info("terminating daemon process");
            state_->manager_.clear();
            state_.reset();
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace augas;

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);
        try {
            service serv;
            program_ = argv[0];

            blocksignals();
            aug_setloglevel(AUG_LOGINFO);
            return main(serv, argc, argv);

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
