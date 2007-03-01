/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augas/clntconn.hpp"
#include "augas/exception.hpp"
#include "augas/listener.hpp"
#include "augas/manager.hpp"
#include "augas/module.hpp"
#include "augas/options.hpp"
#include "augas/servconn.hpp"
#include "augas/utility.hpp"

#if !defined(_WIN32)
# include "augconfig.h"
#else // _WIN32
# define PACKAGE_BUGREPORT "mark@emantic.co.uk"
#endif // _WIN32

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <sstream>
#include <queue>
#include <vector>

#include <time.h>

using namespace aug;
using namespace std;

#define AUGAS_WAKEUP   (AUG_EVENTUSER + 0)
#define AUGAS_MODEVENT (AUG_EVENTUSER + 1)

namespace augas {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    cstring logdir_ = "";
    options options_;
    bool daemon_(false);
    bool stopping_(false);

    void
    openlog_()
    {
        // The current date is appended to the log file name.

        tm tm;
        aug::gmtime(tm);
        stringstream ss;
        ss << "augasd-" << setfill('0')
           << setw(4) << tm.tm_year + 1900
           << setw(2) << tm.tm_mon + 1
           << setw(2) << tm.tm_mday;
        openlog(makepath(logdir_, ss.str().c_str(), "log").c_str());
    }

    void
    doreconf_()
    {
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

        AUG_DEBUG2("loglevel=[%d]", aug_loglevel());
        AUG_DEBUG2("rundir=[%s]", rundir_);
    }

    typedef map<int, servptr> idtoserv;
    typedef queue<connptr> connected;

    struct event : public augas_event {
        string from_, to_;
        void (*destroy_)(void*);
        ~event() AUG_NOTHROW
        {
            if (destroy_)
                destroy_(user_);
        }
        event(const string& from, const string& to, const augas_event& event)
            : from_(from),
              to_(to),
              destroy_(0)
        {
            memcpy(this, &event, sizeof(augas_event));
        }
    };

    struct state {

        filecb_base& cb_;
        mplexer mplexer_;
        manager manager_;
        aug::files files_;
        timers timers_;

        // Mapping of timer-ids to services.

        idtoserv idtoserv_;

        // Pending calls to connected().

        connected connected_;

        explicit
        state(filecb_base& cb)
            : cb_(cb)
        {
            AUG_DEBUG2("adding event pipe to list");
            insertfile(files_, aug_eventin(), cb);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);
        }
    };

    auto_ptr<state> state_;
    const aug_driver* base_(0);

    int
    extclose_(int fd)
    {
        AUG_DEBUG2("clearing io-event mask prior to close: fd=[%d]", fd);
        aug_setioeventmask(state_->mplexer_, fd, 0);
        return base_->close_(fd); // Delegate to base version.
    }

    void
    setextdriver_(fdref ref, filecb_base& cb, unsigned short mask)
    {
        // Override close() function.

        static aug_driver extended = { extclose_, 0, 0, 0, 0, 0 };
        if (!base_) {
            base_ = &getdriver(ref);
            extdriver(extended, *base_);
        }

        AUG_DEBUG2("adding file to list: fd=[%d]", ref.get());
        insertfile(state_->files_, ref, cb);

        try {
            setioeventmask(state_->mplexer_, ref, mask);
            setdriver(ref, extended);
        } catch (...) {
            AUG_DEBUG2("removing file from list: fd=[%d]", ref.get());
            removefile(state_->files_, ref);
            throw;
        }
    }

    void
    timercb_(int id, const aug_var* arg, unsigned* ms, aug_timers* timers)
    {
        AUG_DEBUG2("custom timer expiry");

        idtoserv::iterator it(state_->idtoserv_.find(id));
        servptr serv(it->second);
        augas_object timer = { id, aug_getvarp(arg) };
        serv->expire(timer, *ms);

        if (0 == *ms) {
            AUG_DEBUG2("removing timer: ms has been set to zero");
            state_->idtoserv_.erase(it);
        }
    }

    void
    setsockopts_(const smartfd& sfd)
    {
        setnodelay(sfd, true);
        setnonblock(sfd, true);
    }

    void
    connected_(conn_base& conn)
    {
        setsockopts_(conn.sfd());

        const endpoint& ep(conn.endpoint());
        inetaddr addr(null);
        AUG_DEBUG2("connected: host=[%s], port=[%d]",
                   inetntop(getinetaddr(ep, addr)).c_str(),
                   static_cast<int>(ntohs(port(ep))));

        setioeventmask(state_->mplexer_, conn.sfd(), AUG_IOEVENTRD);
        conn.connected(ep);
    }

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
    reconf_()
    {
        try {
            aug_event e = { AUG_EVENTRECONF, AUG_VARNULL };
            writeevent(aug_eventout(), e);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    stopall_()
    {
        try {
            aug_event e = { AUG_EVENTSTOP, AUG_VARNULL };
            writeevent(aug_eventout(), e);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    // Thread-safe.

    int
    post_(const char* to, const augas_event* event, void (*destroy)(void*))
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("post(): sname=[%s], to=[%s], type=[%s], size=[%d]",
                   sname, to, event->type_, event->size_);
        try {

            auto_ptr<augas::event> arg(new augas::event(sname, to, *event));

            aug_event e;
            e.type_ = AUGAS_MODEVENT;
            aug_setvarp(&e.arg_, arg.get(), 0);
            writeevent(aug_eventout(), e);
            arg->destroy_ = destroy;
            arg.release();
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    dispatch_(const char* to, const augas_event* event)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("dispatch(): sname=[%s], to=[%s], type=[%s], size=[%d]",
                   sname, to, event->type_, event->size_);
        try {

            vector<servptr> servs;
            state_->manager_.getservs(servs, to);

            vector<servptr>::const_iterator it(servs.begin()),
                end(servs.end());
            for (; it != end; ++it)
                (*it)->event(sname, *event);

            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const char*
    getenv_(const char* name, const char* def)
    {
        try {

            const char* value(options_.get(name, 0));

            /* Fallback to environment table. */

            if (!value)
                value = getenv(name);

            return value ? value : def;

        } AUG_SETERRINFOCATCH;
        return 0;
    }

    const augas_serv*
    getserv_()
    {
        try {
            return getserv();
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    int
    shutdown_(augas_id cid)
    {
        AUG_DEBUG2("shutdown(): id=[%d]", cid);
        try {
            objectptr sock(state_->manager_.getbyid(cid));
            connptr cptr(smartptr_cast<conn_base>(sock));
            if (null != cptr)
                cptr->shutdown();
            else
                state_->manager_.erase(*sock);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcpconnect_(const char* host, const char* port, void* user)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("tcpconnect(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {

            servptr serv(state_->manager_.getserv(sname));
            connptr cptr(new augas::clntconn(serv, user, state_->timers_,
                                             host, port));

            // Remove on exception.

            scoped_insert si(state_->manager_, cptr);

            if (ESTABLISHED == cptr->phase()) {

                // connected() must only be called after this function has
                // returned.

                setextdriver_(cptr->sfd(), state_->cb_, AUG_IOEVENTRD);
                if (state_->connected_.empty()) {

                    // Schedule an event to ensure that connected() is called
                    // after this function has returned.

                    aug_event e = { AUGAS_WAKEUP, AUG_VARNULL };
                    writeevent(aug_eventout(), e);
                }

                // Add to pending queue.

                state_->connected_.push(cptr);

            } else
                setextdriver_(cptr->sfd(), state_->cb_, AUG_IOEVENTALL);

            si.commit();
            return (int)cptr->id();

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    tcplisten_(const char* host, const char* port, void* user)
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("tcplisten(): sname=[%s], host=[%s], port=[%s]",
                   sname, host, port);
        try {

            // Bind listener socket.

            endpoint ep(null);
            smartfd sfd(tcplisten(host, port, ep));
            setextdriver_(sfd, state_->cb_, AUG_IOEVENTRD);

            inetaddr addr(null);
            AUG_DEBUG2("listening: interface=[%s], port=[%d]",
                       inetntop(getinetaddr(ep, addr)).c_str(),
                       static_cast<int>(ntohs(aug::port(ep))));

            // Prepare state.

            servptr serv(state_->manager_.getserv(sname));
            listenerptr lptr(new augas::listener(serv, user, sfd));
            scoped_insert si(state_->manager_, lptr);

            si.commit();
            return (int)lptr->id();

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    send_(augas_id cid, const char* buf, size_t size)
    {
        AUG_DEBUG2("send(): id=[%d]", cid);
        try {
            if (!state_->manager_.send(state_->mplexer_, cid, buf, size))
                throw error(__FILE__, __LINE__, EHOSTCALL,
                            "connection has been shutdown");
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("setrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            rwtimer->setrwtimer(ms, flags);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resetrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        AUG_DEBUG2("resetrwtimer(): id=[%d], ms=[%u], flags=[%x]",
                   cid, ms, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            return rwtimer->resetrwtimer(ms, flags) ? 0 : AUGAS_NONE;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    cancelrwtimer_(augas_id cid, unsigned flags)
    {
        AUG_DEBUG2("cancelrwtimer(): id=[%d], flags=[%x]", cid, flags);
        try {
            rwtimerptr rwtimer(smartptr_cast<
                               rwtimer_base>(state_->manager_.getbyid(cid)));
            if (null == rwtimer)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection not found: id=[%d]", cid);
            return rwtimer->cancelrwtimer(flags) ? 0 : AUGAS_NONE;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    settimer_(unsigned ms, void* arg, void (*destroy)(void*))
    {
        const char* sname = getserv()->name_;
        AUG_DEBUG2("settimer(): sname=[%s], ms=[%u]", sname, ms);
        try {

            augas_id id(aug_nextid());
            var v(arg, destroy);

            // If aug_settimer() succeeds, it will call aug_destroyvar() on
            // var when the timer is destroyed.  The service is added to the
            // container first to minimise any chance of failure after
            // aug_settimer() has been called.

            state_->idtoserv_[id] = state_->manager_.getserv(sname);
            if (-1 == aug_settimer(cptr(state_->timers_), id, ms, timercb_,
                                   cptr(v)))
                state_->idtoserv_.erase(id);

            return id;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resettimer_(augas_id tid, unsigned ms)
    {
        AUG_DEBUG2("resettimer(): id=[%d], ms=[%u]", tid, ms);
        try {
            return aug_resettimer(cptr(state_->timers_), tid, ms);
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    canceltimer_(augas_id tid)
    {
        AUG_DEBUG2("canceltimer(): id=[%d]", tid);
        try {
            int ret(aug_canceltimer(cptr(state_->timers_), tid));

            // Only erase if aug_canceltimer() returns true: may be in the
            // midst of a aug_foreachexpired() call, in which case,
            // aug_canceltimer() will return false for the timer being
            // expired.

            if (0 == ret)
                state_->idtoserv_.erase(tid);
            return ret;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const struct augas_host host_ = {
        writelog_,
        vwritelog_,
        error_,
        reconf_,
        stopall_,
        post_,
        dispatch_,
        getenv_,
        getserv_,
        shutdown_,
        tcpconnect_,
        tcplisten_,
        send_,
        setrwtimer_,
        resetrwtimer_,
        cancelrwtimer_,
        settimer_,
        resettimer_,
        canceltimer_
    };

    void
    load_(filecb_base& cb)
    {
        AUG_DEBUG2("loading services");
        state_->manager_.load(rundir_, options_, host_);

        // Remove any timers allocated to services that could not be opened.

        idtoserv::iterator it(state_->idtoserv_.begin()),
            end(state_->idtoserv_.end());
        while (it != end) {
            if (!it->second->active()) {
                aug_warn("cancelling timer associated with inactive service");
                aug_canceltimer(cptr(state_->timers_), it->first);
                state_->idtoserv_.erase(it++);
            } else
                ++it;
        }
    }

    void
    accept_(const object_base& sock, filecb_base& filecb)
    {
        endpoint ep(null);

        AUG_DEBUG2("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(sock.sfd(), ep);

        } catch (const errinfo_error& e) {

            if (aug_acceptlost()) {
                aug_warn("accept() failed: %s", e.what());
                return;
            }
            throw;
        }

        setextdriver_(sfd, state_->cb_, AUG_IOEVENTRD);
        setsockopts_(sfd);
        connptr cptr(new augas::servconn(sock.serv(), sock.user(),
                                         state_->timers_, sfd, ep));

        scoped_insert si(state_->manager_, cptr);
        AUG_DEBUG2("initialising connection: id=[%d], fd=[%d]", cptr->id(),
                   sfd.get());

        if (cptr->accept(ep))
            si.commit();
    }

    bool
    process_(const connptr& cptr, int fd)
    {
        bool changed, ok = false;
        try {
            changed = cptr->process(state_->mplexer_);
            ok = true;
        } AUG_PERRINFOCATCH;

        if (!ok) {

            // Connection is closed if an exception is thrown during
            // processing.

            state_->manager_.erase(*cptr);
            return false;
        }

        if (CONNECTING == cptr->phase()) {

            // The associated file descriptor may change as connection
            // attempts fail and alternative addresses are tried.

            setextdriver_(cptr->sfd(), state_->cb_, AUG_IOEVENTALL);
            state_->manager_.update(cptr, fd);

        } else if (changed)

            switch (cptr->phase()) {
            case ESTABLISHED:

                // Was connecting, now established: notify module of
                // connection establishment.

                connected_(*cptr);
                break;
            case CLOSED:
                state_->manager_.erase(*cptr);
                return false;
            default:
                break;
            }

        return true;
    }

    bool
    readevent_(filecb_base& cb)
    {
        aug_event event;
        AUG_DEBUG2("reading event");

        switch (aug::readevent(aug_eventin(), event).type_) {
        case AUG_EVENTRECONF:
            AUG_DEBUG2("received AUG_EVENTRECONF");
            if (*conffile_) {
                AUG_DEBUG2("reading config-file: path=[%s]", conffile_);
                options_.read(conffile_);
            }
            doreconf_();
            state_->manager_.reconf();
            break;
        case AUG_EVENTSTATUS:
            AUG_DEBUG2("received AUG_EVENTSTATUS");
            break;
        case AUG_EVENTSTOP:
            AUG_DEBUG2("received AUG_EVENTSTOP");
            stopping_ = true;
            state_->manager_.teardown();
            break;
        case AUG_EVENTSIGNAL:
            AUG_DEBUG2("received AUG_EVENTSIGNAL");
            break;
        case AUGAS_WAKEUP:
            AUG_DEBUG2("received AUGAS_WAKEUP");
            // Actual handling is performed in do_run().
            break;
        case AUGAS_MODEVENT:
            assert(AUG_VTPTR == event.arg_.type_);
            {
                auto_ptr<augas::event> arg
                    (static_cast<augas::event*>(aug_getvarp(&event.arg_)));

                vector<servptr> servs;
                state_->manager_.getservs(servs, arg->to_);

                vector<servptr>::const_iterator it(servs.begin()),
                    end(servs.end());
                for (; it != end; ++it)
                    (*it)->event(arg->from_.c_str(), *arg);
            }
        }
        aug_destroyvar(&event.arg_);
        return true;
    }

    class service : public filecb_base, public service_base {

        bool
        do_callback(int fd, aug_files& files)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            // Intercept activity on event pipe.

            if (fd == aug_eventin())
                return readevent_(*this);

            objectptr sock(state_->manager_.getbyfd(fd));
            connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

            AUG_DEBUG2("processing sock: id=[%d], fd=[%d]", sock->id(), fd);

            if (null != cptr)
                return process_(cptr, fd);

            accept_(*sock, *this);
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

            doreconf_();
        }

        void
        do_init()
        {
            AUG_DEBUG2("initialising daemon process");

            setsrvlogger("augasd");

            auto_ptr<state> s(new state(*this));
            state_ = s;
            try {
                load_(*this);
            } catch (...) {

                // Ownership back to local.

                s = state_;
                throw;
            }
        }

        void
        do_run()
        {
            struct timercb : timercb_base {
                void
                do_callback(idref ref, unsigned& ms, aug_timers& timers)
                {
                    AUG_DEBUG2("re-opening log file");
                    openlog_();
                }
            } cb;
            timer reopen(state_->timers_);

            // Re-open log file every minute.

            if (daemon_)
                reopen.set(60000, cb);

            AUG_DEBUG2("running daemon process");

            int ret(!0);
            while (!stopping_ || !state_->manager_.empty()) {

                try {

                    if (state_->timers_.empty()) {

                        scoped_unblock unblock;
                        while (AUG_RETINTR == (ret = waitioevents
                                               (state_->mplexer_)))
                            ;

                    } else {

                        AUG_DEBUG2("processing timers");

                        struct timeval tv;
                        foreachexpired(state_->timers_, 0 == ret, tv);

                        scoped_unblock unblock;
                        while (AUG_RETINTR == (ret = waitioevents
                                               (state_->mplexer_, tv)))
                            ;
                    }

                    // Notify of any established connections before processing
                    // the files: data may have arrived on a newly established
                    // connection.

                    while (!state_->connected_.empty()) {
                        connected_(*state_->connected_.front());
                        state_->connected_.pop();
                    }

                    AUG_DEBUG2("processing files");

                    foreachfile(state_->files_);
                    continue;

                } AUG_PERRINFOCATCH;

                if (!stopping_ && !daemon_) {
                    stopping_ = true;
                    state_->manager_.teardown();
                }
            }
        }

        void
        do_term()
        {
            AUG_DEBUG2("terminating daemon process");

            // Clear services first.

            state_->manager_.clear();

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

        } AUG_PERRINFOCATCH;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
