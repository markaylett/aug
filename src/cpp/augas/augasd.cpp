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

    struct session : public timercb_base {

        augas_session session_;
        smartfd sfd_;
        moduleptr module_;
        mplexer& mplexer_;
        timer rdtimer_;
        timer wrtimer_;
        buffer buffer_;
        bool shutdown_;

        void
        do_callback(idref ref, unsigned& ms, aug_timers& timers)
        {
            if (rdtimer_ == ref) {
                aug_info("read timer expiry");
                module_->rdexpire(session_, ms);
            } else if (wrtimer_ == ref) {
                aug_info("write timer expiry");
                module_->wrexpire(session_, ms);
            } else
                assert(0);
        }
        ~session() AUG_NOTHROW
        {
            try {
                module_->close(session_);
                setioeventmask(mplexer_, sfd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(augas_sid sid, const smartfd& sfd, const char* serv,
                const aug_endpoint& ep, const moduleptr& module,
                mplexer& mplexer, timers& timers)
            : sfd_(sfd),
              module_(module),
              mplexer_(mplexer),
              rdtimer_(timers, null),
              wrtimer_(timers, null),
              shutdown_(false)
        {
            session_.sid_ = sid;
            session_.user_ = 0;

            inetaddr addr(null);
            module->open(session_, serv,
                         inetntop(getinetaddr(ep, addr)).c_str());
            session_.sid_ = sid; // For safety: the callee may have changed.
        }
    };

    typedef smartptr<session> sessionptr;
    typedef map<int, augas_sid> sids;
    typedef map<augas_sid, sessionptr> sessions;
    typedef map<int, moduleptr> pending;
    typedef map<unsigned, pair<moduleptr, void*> > events;

    struct state {

        mutex mutex_;
        conns conns_;
        timers timers_;
        mplexer mplexer_;
        manager manager_;
        sids sids_;
        sessions sessions_;
        pending pending_;
        events events_;
        string lastError_;

        explicit
        state(conncb_base& cb)
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
    timercb_(const aug_var* arg, int id, unsigned* ms, aug_timers* timers)
    {
        aug_info("custom timer expiry");

        pending::iterator it(state_->pending_.find(id));
        moduleptr ptr(it->second);
        ptr->expire(aug_getvarp(arg), id, ms);
        if (0 == *ms) // What if canceltimer is used?
            state_->pending_.erase(it);
    }

    const char*
    error_(const char* modname)
    {
        return state_->lastError_.c_str();
    }

    const char*
    getenv_(const char* modname, const char* name)
    {
        return options_.get(string(modname).append(".").append(name)
                            .c_str(), 0);
    }

    void
    writelog_(const char* modname, int level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        aug_vwritelog(level, format, args);
        va_end(args);
    }

    void
    vwritelog_(const char* modname, int level, const char* format,
               va_list args)
    {
        aug_vwritelog(level, format, args);
    }

    int
    post_(const char* modname, int type, void* arg)
    {
        unsigned id(aug_nextid());

        {
            scoped_lock l(state_->mutex_);
            moduleptr ptr(getmodule(state_->manager_.modules_, modname));
            state_->events_[id] = make_pair(ptr, arg);
        }

        aug_event e;
        e.type_ = type + AUG_EVENTUSER;
        aug_setvarl(&e.arg_, id);
        writeevent(aug_eventout(), e);
        return 0;
    }

    int
    settimer_(const char* modname, int id, unsigned ms, void* arg)
    {
        var v(arg);
        id = aug_settimer(cptr(state_->timers_), id, ms, timercb_, cptr(v));
        if (0 < id) {
            scoped_lock l(state_->mutex_);
            state_->pending_[id] = getmodule(state_->manager_.modules_,
                                             modname);
        }
        return id;
    }

    int
    resettimer_(const char* modname, int id, unsigned ms)
    {
        int ret(aug_resettimer(cptr(state_->timers_), id, ms));
        if (ret < 0)
            state_->pending_.erase(id);
        return ret;
    }

    int
    canceltimer_(const char* modname, int id)
    {
        int ret(aug_canceltimer(cptr(state_->timers_), id));
        state_->pending_.erase(id);
        return ret;
    }

    int
    shutdown_(augas_sid sid)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {
            sessionptr ptr(it->second);
            ptr->shutdown_ = true;
            if (ptr->buffer_.empty())
                aug::shutdown(ptr->sfd_, SHUT_WR);
        }
        return 0;
    }

    int
    sendall_(augas_sid sid, const char* buf, size_t size)
    {
        int ret(0);
        sessions::const_iterator it(state_->sessions_.begin()),
            end(state_->sessions_.end());
        for (; it != end; ++it) {
            if (it->second->shutdown_) {
                if (it->second->session_.sid_ == sid) {
                    state_->lastError_ = "session has been shutdown";
                    ret = -1;
                }
                continue;
            }
            it->second->buffer_.putsome(buf, size);
            setioeventmask(state_->mplexer_, it->second->sfd_,
                           AUG_IOEVENTRDWR);
        }
        return ret;
    }

    int
    sendself_(augas_sid sid, const char* buf, size_t size)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {
            if (it->second->shutdown_) {
                state_->lastError_ = "session has been shutdown";
                return -1;
            }
            it->second->buffer_.putsome(buf, size);
            setioeventmask(state_->mplexer_, it->second->sfd_,
                           AUG_IOEVENTRDWR);
        }
        return 0;
    }

    int
    sendother_(augas_sid sid, const char* buf, size_t size)
    {
        sessions::const_iterator it(state_->sessions_.begin()),
            end(state_->sessions_.end());
        for (; it != end; ++it) {

            // Ignore self and sessions that have been marked for shutdown.

            if (it->second->session_.sid_ == sid
                || it->second->shutdown_)
                continue;

            it->second->buffer_.putsome(buf, size);
            setioeventmask(state_->mplexer_, it->second->sfd_,
                           AUG_IOEVENTRDWR);
        }
        return 0;
    }

    int
    send_(augas_sid sid, const char* buf, size_t size, unsigned flags)
    {
        switch (flags) {
        case AUGAS_SESALL:
            return sendall_(sid, buf, size);
        case AUGAS_SESSELF:
            return sendself_(sid, buf, size);
        case AUGAS_SESOTHER:
            return sendother_(sid, buf, size);
        }

        state_->lastError_ = "invalid flags";
        return -1;
    }

    int
    setrwtimer_(augas_sid sid, unsigned ms, unsigned flags)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {

            if (flags & AUGAS_TIMRD)
                it->second->rdtimer_.set(ms, *it->second);

            if (flags & AUGAS_TIMWR)
                it->second->wrtimer_.set(ms, *it->second);
        }
        return 0;
    }

    int
    resetrwtimer_(augas_sid sid, unsigned ms, unsigned flags)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {

            if (flags & AUGAS_TIMRD)
                it->second->rdtimer_.reset(ms);

            if (flags & AUGAS_TIMWR)
                it->second->wrtimer_.reset(ms);
        }
        return 0;
    }

    int
    cancelrwtimer_(augas_sid sid, unsigned flags)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {

            if (flags & AUGAS_TIMRD)
                it->second->rdtimer_.cancel();

            if (flags & AUGAS_TIMWR)
                it->second->wrtimer_.cancel();
        }
        return 0;
    }

    const struct augas_host host_ = {
        error_,
        getenv_,
        writelog_,
        vwritelog_,
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
        aug_info("loading modules");
        state_->manager_.load(options_, host_);

        aug_info("adding listener sockets");

        map<int, serviceinfo>::const_iterator
            it(state_->manager_.services_.begin()),
            end(state_->manager_.services_.end());
        for (; it != end; ++it)
            setextdriver(it->second.sfd_, cb, AUG_IOEVENTRD);
    }

    bool
    listener(const serviceinfo& si, conncb_base& conncb)
    {
        aug_endpoint ep;

        AUG_DEBUG("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(si.sfd_, ep);

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

        unsigned sid(aug_nextid());
        state_->sids_[sfd.get()] = sid;
        state_->sessions_.insert
            (make_pair(sid, sessionptr
                       (new session(sid, sfd, si.name_.c_str(), ep,
                                    si.module_, state_->mplexer_,
                                    state_->timers_))));

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
                reconf(state_->manager_.modules_);
            }
            break;
        case AUG_EVENTSTATUS:
            aug_info("received AUG_EVENTSTATUS");
            break;
        case AUG_EVENTSTOP:
            aug_info("received AUG_EVENTSTOP");
            {
                sessions::const_iterator it(state_->sessions_.begin()),
                    end(state_->sessions_.end());
                for (; it != end; ++it)
                    it->second->module_->stop(it->second->session_);
            }
            stopping_ = true;
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

    bool
    connection(int fd)
    {
        sessionptr ptr(state_->sessions_[state_->sids_[fd]]);
        unsigned short bits(ioevents(state_->mplexer_, fd));

        if (bits & AUG_IOEVENTRD) {

            AUG_DEBUG("handling read event '%d'", fd);

            char buf[4096];
            size_t size(aug::read(fd, buf, sizeof(buf)));
            if (0 == size) {

                // Connection closed.

                aug_info("closing connection '%d'", fd);
                state_->sessions_.erase(state_->sids_[fd]);
                state_->sids_.erase(fd);
                return false;
            }

            // Data has been read: reset read timer.

            if (null != ptr->rdtimer_)
                if (!ptr->rdtimer_.reset()) // If timer nolonger exists.
                    ptr->rdtimer_ = null;

            // Notify module of new data.

            ptr->module_->data(ptr->session_, buf, size);
        }

        if (bits & AUG_IOEVENTWR) {

            bool more(ptr->buffer_.writesome(fd));

            // Data has been written: reset write timer.

            if (null != ptr->wrtimer_)
                if (!ptr->wrtimer_.reset()) // If timer nolonger exists.
                    ptr->wrtimer_ = null;

            if (!more) {

                // No more (buffered) data to be written.

                setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRD);

                // If flagged for shutdown, send FIN and disable writes.

                if (ptr->shutdown_)
                    aug::shutdown(ptr->sfd_, SHUT_WR);
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

            services::const_iterator it(state_->manager_.services_.find(fd));
            if (it != state_->manager_.services_.end())
                return listener(it->second, *this);

            return connection(fd);
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
            while (!stopping_ || !state_->sessions_.empty()) {

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
            state_->manager_.services_.clear();
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
