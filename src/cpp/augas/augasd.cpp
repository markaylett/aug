/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "module.h"

#include <cassert>
#include <map>
#include <memory> // auto_ptr<>
#include <vector>

#include <time.h>

using namespace aug;
using namespace std;

namespace augas {

    void
    close_(const struct augas_session* s)
    {
    }

    int
    open_(struct augas_session* s, const char* serv)
    {
        return 0;
    }

    int
    data_(const struct augas_session* s, const char* buf, size_t size)
    {
        return 0;
    }

    int
    rdexpire_(const struct augas_session* s, unsigned* ms)
    {
        return 0;
    }

    int
    wrexpire_(const struct augas_session* s, unsigned* ms)
    {
        return 0;
    }

    int
    stop_(const struct augas_session* s)
    {
        return 0;
    }

    int
    event_(int type, void* arg)
    {
        return 0;
    }

    int
    expire_(void* arg, unsigned id, unsigned* ms)
    {
        return 0;
    }

    int
    reconf_(void)
    {
        return 0;
    }

    void
    setdefaults(struct augas_module& dst, const struct augas_module& src)
    {
        dst.close_ = src.close_ ? src.close_ : close_;
        dst.open_ = src.open_ ? src.open_ : open_;
        dst.data_ = src.data_ ? src.data_ : data_;
        dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire_;
        dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire_;
        dst.stop_ = src.stop_ ? src.stop_ : stop_;
        dst.event_ = src.event_ ? src.event_ : event_;
        dst.expire_ = src.expire_ ? src.expire_ : expire_;
        dst.reconf_ = src.reconf_ ? src.reconf_ : reconf_;
    }

    class module {
        dlib lib_;
        augas_unloadfn unloadfn_;
        struct augas_module module_;
    public:
        ~module() AUG_NOTHROW
        {
            unloadfn_();
        }
        module(const char* path, const struct augas_service& service)
            : lib_(path)
        {
            augas_loadfn loadfn(dlsym<augas_loadfn>(lib_, "augas_load"));
            unloadfn_ = dlsym<augas_unloadfn>(lib_, "augas_unload");
            setdefaults(module_, *loadfn(&service));
        }
        void
        close(const struct augas_session& s) const
        {
            module_.close_(&s);
        }
        int
        open(struct augas_session& s, const char* serv) const
        {
            return module_.open_(&s, serv);
        }
        int
        data(const struct augas_session& s, const char* buf,
             size_t size) const
        {
            return module_.data_(&s, buf, size);
        }
        int
        rdexpire(const struct augas_session& s, unsigned& ms) const
        {
            return module_.rdexpire_(&s, &ms);
        }
        int
        wrexpire(const struct augas_session& s, unsigned& ms) const
        {
            return module_.wrexpire_(&s, &ms);
        }
        int
        stop(const struct augas_session& s) const
        {
            return module_.stop_(&s);
        }
        int
        event(int type, void* arg) const
        {
            return module_.event_(type, arg);
        }
        int
        expire(void* arg, unsigned id, unsigned* ms) const
        {
            return module_.expire_(arg, id, ms);
        }
        int
        reconf() const
        {
            return module_.reconf_();
        }
    };

    class buffer {
        vector<char> vec_;
        size_t begin_, end_;
    public:
        explicit
        buffer(size_t size = 4096)
            : vec_(size),
              begin_(0),
              end_(0)
        {
        }
        void
        putsome(const void* buf, size_t size)
        {
            if (vec_.size() - end_ < size)
                vec_.resize(end_ + size);

            memcpy(&vec_[end_], buf, size);
            end_ += size;
        }
        bool
        readsome(fdref ref)
        {
            char buf[4096];
			size_t size(aug::read(ref, buf, sizeof(buf) - 1));
            if (0 == size)
                return false;

            putsome(buf, size);
            return true;
        }
        bool
        writesome(fdref ref)
        {
            size_t size(end_ - begin_);
			size = aug::write(ref, &vec_[begin_], size);
            if ((begin_ += size) == end_) {
                begin_ = end_ = 0;
                return false;
            }
            return true;
        }
        bool
        consume(size_t n)
        {
            size_t size(end_ - begin_);
            size = AUG_MIN(n, size);
            if ((begin_ += size) == end_) {
                begin_ = end_ = 0;
                return false;
            }
            return true;
        }
        bool
        empty() const
        {
            return begin_ == end_;
        }
    };

    class opts : private confcb_base {
        map<string, string> opts_;
        void
        do_callback(const char* name, const char* value)
        {
            opts_[name] = value;
        }
    public:
        ~opts() AUG_NOTHROW
        {
        }
        void
        read(const char* path)
        {
            opts_.clear();
            readconf(path, *this);
        }
        void
        set(const string& name, const string& value)
        {
            opts_[name] = value;
        }
        const char*
        get(const string& name, const char* def = 0) const
        {
            map<string, string>::const_iterator it(opts_.find(name));
            if (opts_.find(name) == opts_.end())
                return def;
            return it->second.c_str();
        }
    };

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    opts opts_;
    bool daemon_(false);
    bool stopping_(false);

    void
    reconf()
    {
        const char* loglevel(opts_.get("loglevel", 0));
        if (loglevel) {
            unsigned level(strtoui(loglevel, 10));
            aug_info("setting log level: %d", level);
            aug_setloglevel(level);
        }

        aug::chdir(rundir_);
        if (daemon_)
            openlog(opts_.get("logfile", "augasd.log"));

        aug_info("log level: %d", aug_loglevel());
        aug_info("run directory: %s", rundir_);
    }

    struct session : public timercb_base {

        struct augas_session session_;
        smartfd sfd_;
        module& module_;
        mplexer& mplexer_;
        timer rdtimer_;
        timer wrtimer_;
        buffer buffer_;
        bool shutdown_;

        void
        do_callback(idref ref, unsigned& ms, struct aug_timers& timers)
        {
            aug_info("timeout");
            if (rdtimer_ == ref)
                module_.rdexpire(session_, ms);
            else if (wrtimer_ == ref)
                module_.wrexpire(session_, ms);
            else
                assert(0);
        }
        ~session() AUG_NOTHROW
        {
            try {
                module_.close(session_);
                setioeventmask(mplexer_, sfd_, 0);
            } AUG_PERRINFOCATCH;
        }
        session(augas_sid sid, const smartfd& sfd, module& module,
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
            module.open(session_, "augas");
            session_.sid_ = sid; // For safety.
        }
    };

    typedef smartptr<session> sessionptr;
    typedef map<int, augas_sid> sids;
    typedef map<augas_sid, sessionptr> sessions;

    struct state {

        conns conns_;
        timers timers_;
        mplexer mplexer_;
        smartfd sfd_;
        sids sids_;
        sessions sessions_;
        string lastError_;
        module module_;

        explicit
        state(const struct augas_service& service, conncb_base& cb)
            : sfd_(null),
              module_("module.so", service)
        {
            aug_hostserv hostserv;
            parsehostserv(opts_.get("address", "127.0.0.1:8080"), hostserv);

            endpoint ep(null);
            smartfd sfd(tcplisten(hostserv.host_, hostserv.serv_, ep));

            // Add event pipe.

            insertconn(conns_, aug_eventin(), cb);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);

            // Add listener socket.

            insertconn(conns_, sfd, cb);
            setioeventmask(mplexer_, sfd, AUG_IOEVENTRD);

            sfd_ = sfd;
        }
    };

    auto_ptr<state> state_;

    void
    timercb_(const struct aug_var* arg, int id, unsigned* ms,
             struct aug_timers* timers)
    {
        state_->module_.expire(aug_getvarp(arg), id, ms);
    }

    const char*
    error_(void)
    {
        return state_->lastError_.c_str();
    }

    const char*
    getenv_(const char* name)
    {
        return opts_.get(name, 0);
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

    int
    post_(int type, void* arg)
    {
        struct aug_event e;
        e.type_ = type;
        aug_setvarp(&e.arg_, arg);
        writeevent(aug_eventout(), e);
        return 0;
    }

    int
    settimer_(int id, unsigned ms, void* arg)
    {
        var v(arg);
        aug_settimer(cptr(state_->timers_), 0, ms, timercb_, cptr(v));
        return 0;
    }

    int
    resettimer_(int id, unsigned ms)
    {
        aug_resettimer(cptr(state_->timers_), id, ms);
        return 0;
    }

    int
    canceltimer_(int id)
    {
        aug_canceltimer(cptr(state_->timers_), id);
        return 0;
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
    sendall_(augas_sid sid, const char* buf, unsigned size)
    {
        int ret(0);
        sessions::const_iterator it(state_->sessions_.begin()),
            end(state_->sessions_.end());
        for (; it != end; ++it) {
            if (it->second->shutdown_) {
                if (it->second->session_.sid_ == sid)
                    ret = -1;
                continue;
            }
            it->second->buffer_.putsome(buf, size);
            setioeventmask(state_->mplexer_, it->second->sfd_,
                           AUG_IOEVENTRDWR);
        }
        return ret;
    }

    int
    sendself_(augas_sid sid, const char* buf, unsigned size)
    {
        sessions::const_iterator it(state_->sessions_.find(sid));
        if (it != state_->sessions_.end()) {
            if (it->second->shutdown_)
                return -1;
            it->second->buffer_.putsome(buf, size);
            setioeventmask(state_->mplexer_, it->second->sfd_,
                           AUG_IOEVENTRDWR);
        }
        return 0;
    }

    int
    sendother_(augas_sid sid, const char* buf, unsigned size)
    {
        sessions::const_iterator it(state_->sessions_.begin()),
            end(state_->sessions_.end());
        for (; it != end; ++it) {

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
    send_(augas_sid sid, const char* buf, unsigned size, unsigned flags)
    {
        switch (flags) {
        case AUGAS_SESALL:
            return sendall_(sid, buf, size);
        case AUGAS_SESSELF:
            return sendself_(sid, buf, size);
        case AUGAS_SESOTHER:
            return sendother_(sid, buf, size);
        }
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

    const struct augas_service fntable_ = {
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
    setfdhook(fdref ref, conncb_base& conncb, unsigned short mask)
    {
        insertconn(state_->conns_, ref, conncb);
        try {
            setioeventmask(state_->mplexer_, ref, mask);
        } catch (...) {
            removeconn(state_->conns_, ref);
        }
    }

    bool
    listener(conncb_base& conncb)
    {
        struct aug_endpoint ep;

        AUG_DEBUG("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(state_->sfd_, ep);

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
        setfdhook(sfd, conncb, AUG_IOEVENTRD);

        unsigned sid(aug_nextid());
        state_->sids_[sfd.get()] = sid;
        state_->sessions_.insert
            (make_pair(sid, sessionptr
                       (new session(sid, sfd, state_->module_,
                                    state_->mplexer_, state_->timers_))));
        return true;
    }

    bool
    readevent()
    {
        struct aug_event event;
        AUG_DEBUG("reading event");

        switch (aug::readevent(aug_eventin(), event).type_) {
        case AUG_EVENTRECONF:
            aug_info("received AUG_EVENTRECONF");
            if (*conffile_) {
                aug_info("reading: %s", conffile_);
                opts_.read(conffile_);
            }
            reconf();
            state_->module_.reconf();
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
                    state_->module_.stop(it->second->session_);
            }
            stopping_ = true;
            break;
        default:
            assert(AUG_VTLONG != event.arg_.type_);
            state_->module_.event(event.type_, event.arg_.un_.ptr_);
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
			unsigned size((unsigned)aug::read(fd, buf, sizeof(buf)));
            if (0 == size) {
                aug_info("closing connection '%d'", fd);
                state_->sessions_.erase(state_->sids_[fd]);
                state_->sids_.erase(fd);
                return false;
            }

            ptr->module_.data(ptr->session_, buf, size);

            setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRDWR);
            ptr->wrtimer_.cancel();
        }

        if (bits & AUG_IOEVENTWR) {

            if (!ptr->buffer_.writesome(fd)) {
                setioeventmask(state_->mplexer_, fd, AUG_IOEVENTRD);

                if (ptr->shutdown_)
                    aug::shutdown(ptr->sfd_, SHUT_WR);

                if (null != ptr->wrtimer_)
                    if (!ptr->wrtimer_.reset())
                        ptr->wrtimer_ = null;
            }
        }

        return true;
    }

    class service : public conncb_base, public service_base {

        bool
        do_callback(int fd, struct aug_conns& conns)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            if (fd == aug_eventin())
                return readevent();

            if (fd == state_->sfd_.get())
                return listener(*this);

            return connection(fd);
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
                return "AugAsd Daemon";
            case AUG_OPTPIDFILE:
                return opts_.get("pidfile", "augasd.pid");
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
                opts_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Once set, the run directory should not change.

            const char* rundir(opts_.get("rundir", 0));
            realpath(rundir_, rundir ? rundir : getcwd().c_str(),
                     sizeof(rundir_));

            reconf();
        }

        void
        do_init()
        {
            aug_info("initialising daemon process");

            setsrvlogger("augasd");

            auto_ptr<state> s(new state(fntable_, *this));
            state_ = s;
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
            state_.reset();
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace augas;

    try {

        struct aug_errinfo errinfo;
        scoped_init init(errinfo);
        service serv;

        program_ = argv[0];

        blocksignals();
        aug_setloglevel(AUG_LOGDEBUG);

        main(serv, argc, argv);

    } catch (const exception& e) {

        aug_error("%s", e.what());
    }

    return 1; // aug_main() does not return.
}
