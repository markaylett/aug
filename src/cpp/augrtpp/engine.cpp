/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/engine.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrtpp/clntconn.hpp"
#include "augrtpp/listener.hpp"
#include "augrtpp/servconn.hpp"
#include "augrtpp/sessions.hpp"
#include "augrtpp/socks.hpp"
#include "augrtpp/ssl.hpp"

#include "augsrvpp/signal.hpp"

#include "augnetpp/nbfile.hpp"

#include "augutilpp/timer.hpp"

#include <map>
#include <queue>

#define POSTEVENT_ (AUG_EVENTUSER - 1)

using namespace aug;
using namespace std;

// Definition placed outside anonymous namespace to avoid compiler warnings.

struct sessiontimer {
    sessionptr session_;
    aug_var var_;
    ~sessiontimer() AUG_NOTHROW
    {
        try {
            destroyvar(var_);
        } AUG_PERRINFOCATCH;
    }
    explicit
    sessiontimer(const sessionptr& session)
        : session_(session)
    {
        var_.type_ = 0;
        var_.arg_ = 0;
    }
};

namespace {

    typedef smartptr<sessiontimer> sessiontimerptr;
    typedef map<augrt_id, sessiontimerptr> sessiontimers;

    struct postevent {
        string from_, to_, type_;
        aug_var var_;
        ~postevent() AUG_NOTHROW
        {
            try {
                destroyvar(var_);
            } AUG_PERRINFOCATCH;
        }
        postevent(const string& from, const string& to, const string& type)
            : from_(from),
              to_(to),
              type_(type)
        {
            var_.type_ = 0;
            var_.arg_ = 0;
        }
    };

    typedef std::queue<connptr> pending;

    void
    setsockopts(const smartfd& sfd)
    {
        setnodelay(sfd, true);
        setnonblock(sfd, true);
    }

    void
    setconnected(conn_base& conn)
    {
        // Connection has now been established.

        setsockopts(conn.sfd());

        const endpoint& ep(conn.peername());
        inetaddr addr(null);
        AUG_DEBUG2("connected: host=[%s], port=[%d]",
                   inetntop(getinetaddr(ep, addr)).c_str(),
                   static_cast<int>(ntohs(port(ep))));

        setnbeventmask(conn.sfd(), AUG_FDEVENTRD);

        // Notify session of establishment.

        conn.connected(ep);
    }
}

AUGRTPP_API
enginecb_base::~enginecb_base() AUG_NOTHROW
{
}

namespace aug {

    namespace detail {

        struct engineimpl {

            smartfd eventrd_, eventwr_;
            timers& timers_;
            enginecb_base& cb_;
            nbfiles nbfiles_;
            sessions sessions_;
            socks socks_;

            // Grace period on shutdown.

            timer grace_;

            // Mapping of timer-ids to sessions.

            sessiontimers sessiontimers_;

            // Pending calls to connected().

            pending pending_;

            enum {
                STARTED,
                TEARDOWN,
                STOPPED
            } state_;

            ~engineimpl() AUG_NOTHROW
            {
                AUG_DEBUG2("removing event pipe from list");
                try {
                    removenbfile(eventrd_);
                } AUG_PERRINFOCATCH;
            }
            engineimpl(const smartfd& eventrd, const smartfd& eventwr,
                       timers& timers, enginecb_base& cb)
                : eventrd_(eventrd),
                  eventwr_(eventwr),
                  timers_(timers),
                  cb_(cb),
                  grace_(timers_),
                  state_(STARTED)
            {
                AUG_DEBUG2("inserting event pipe to list");
                insertnbfile(nbfiles_, eventrd_, *this);
                setnbeventmask(eventrd_, AUG_FDEVENTRD);
            }
            void
            teardown()
            {
                if (STARTED == state_) {

                    state_ = TEARDOWN;
                    socks_.teardown();

                    // Initiate grace period.

                    aug_var var = { 0, this };
                    grace_.set(15000, timermemcb<engineimpl,
                               &engineimpl::stopcb>, var);
                }
            }
            void
            accept(const sock_base& sock)
            {
                endpoint ep(null);

                AUG_DEBUG2("accepting connection");

                smartfd sfd(null);
                try {

                    sfd = aug::accept(sock.sfd(), ep);

                } catch (const errinfo_error& e) {

                    if (aug_acceptlost()) {
                        aug_warn("accept() failed: %s", e.what());
                        return;
                    }
                    throw;
                }

                insertnbfile(nbfiles_, sfd, *this);
                setnbeventmask(sfd, AUG_FDEVENTRD);

                setsockopts(sfd);
                connptr cptr(new servconn(sock.session(), user(sock),
                                          timers_, sfd, ep));

                scoped_insert si(socks_, cptr);
                AUG_DEBUG2("initialising connection: id=[%d], fd=[%d]",
                           id(*cptr), sfd.get());

                // Session may reject the connection by returning false.

                if (cptr->accepted(ep))
                    si.commit();
            }
            bool
            process(const connptr& cptr, int fd, unsigned short events)
            {
                bool changed = false, ok = false;
                try {
                    changed = cptr->process(events);
                    ok = true;
                } AUG_PERRINFOCATCH;

                // If an exception was thrown, "ok" will still have its
                // original value of false.

                if (!ok) {

                    // Connection is closed if an exception is thrown during
                    // processing.

                    socks_.erase(*cptr);
                    return false;
                }

                if (HANDSHAKE == cptr->state()) {

                    // The associated file descriptor may change as connection
                    // attempts fail and alternative addresses are tried.

                    insertnbfile(nbfiles_, cptr->sfd(), *this);
                    setnbeventmask(cptr->sfd(), AUG_FDEVENTALL);

                    socks_.update(cptr, fd);

                } else if (changed)

                    switch (cptr->state()) {
                    case CONNECTED:

                        // Was connecting, now established: notify module of
                        // connection establishment.

                        setconnected(*cptr);
                        break;
                    case CLOSED:
                        socks_.erase(*cptr);
                        return false;
                    default:
                        break;
                    }

                return true;
            }
            bool
            readevent()
            {
                aug_event event;
                AUG_DEBUG2("reading event");

                switch (aug::readevent(eventrd_, event).type_) {
                case AUG_EVENTRECONF:
                    AUG_DEBUG2("received AUG_EVENTRECONF");
                    cb_.reconf();
                    sessions_.reconf();
                    break;
                case AUG_EVENTSTATUS:
                    AUG_DEBUG2("received AUG_EVENTSTATUS");
                    break;
                case AUG_EVENTSTOP:
                    AUG_DEBUG2("received AUG_EVENTSTOP");
                    teardown();
                    break;
                case AUG_EVENTSIGNAL:
                    AUG_DEBUG2("received AUG_EVENTSIGNAL");
                    break;
                case AUG_EVENTWAKEUP:
                    AUG_DEBUG2("received AUG_EVENTWAKEUP");
                    // Actual handling is performed in do_run().
                    break;
                case POSTEVENT_:
                    AUG_DEBUG2("received POSTEVENT_");
                    {
                        auto_ptr<postevent> ev
                            (static_cast<postevent*>(event.var_.arg_));

                        vector<sessionptr> sessions;
                        sessions_.getbygroup(sessions, ev->to_);

                        size_t size;
                        const void* user(varbuf(ev->var_, size));

                        vector<sessionptr>
                            ::const_iterator it(sessions.begin()),
                            end(sessions.end());
                        for (; it != end; ++it)
                            (*it)->event(ev->from_.c_str(), ev->type_.c_str(),
                                         user, size);
                    }
                }
                destroyvar(event.var_);
                return true;
            }
            bool
            nbfilecb(int fd, unsigned short events)
            {
                // Intercept activity on event pipe.

                if (fd == eventrd_)
                    return readevent();

                sockptr sock(socks_.getbyfd(fd));
                connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

                AUG_DEBUG2("processing sock: id=[%d], fd=[%d]",
                           id(*sock), fd);

                if (null != cptr)
                    return process(cptr, fd, events);

                accept(*sock);
                return true;
            }
            void
            timercb(int id, unsigned& ms)
            {
                AUG_DEBUG2("custom timer expiry");

                sessiontimers::iterator it(sessiontimers_.find(id));
                sessiontimerptr tptr(it->second);
                augrt_object timer = { id, tptr->var_.arg_ };
                tptr->session_->expire(timer, ms);

                if (0 == ms) {
                    AUG_DEBUG2("removing timer: ms has been set to zero");
                    sessiontimers_.erase(it);
                }
            }
            void
            stopcb(int id, unsigned& ms)
            {
                // Called by grace timer when excessive time has been spent in
                // teardown state.

                aug_info("giving-up, closing connections");
                state_ = STOPPED;
            }
        };
    }
}

AUGRTPP_API
engine::~engine() AUG_NOTHROW
{
    delete impl_;
}

AUGRTPP_API
engine::engine(const smartfd& eventrd, const smartfd& eventwr, timers& timers,
               enginecb_base& cb)
    : impl_(new detail::engineimpl(eventrd, eventwr, timers, cb))
{
}

AUGRTPP_API void
engine::clear()
{
    impl_->socks_.clear();

    // TODO: erase the sessions in reverse order to which they were added.

    impl_->sessions_.clear();
}

AUGRTPP_API void
engine::insert(const string& name, const sessionptr& session,
               const char* groups)
{
    impl_->sessions_.insert(name, session, groups);
}

AUGRTPP_API void
engine::cancelinactive()
{
    // Remove any timers allocated to sessions that could not be opened.

    sessiontimers::iterator it(impl_->sessiontimers_.begin()),
        end(impl_->sessiontimers_.end());
    while (it != end) {
        if (!it->second->session_->active()) {
            aug_warn("cancelling timer associated with inactive session");
            aug_canceltimer(cptr(impl_->timers_), it->first);
            impl_->sessiontimers_.erase(it++);
        } else
            ++it;
    }
}

AUGRTPP_API void
engine::run(bool stoponerr)
{
    AUG_DEBUG2("running daemon process");

    int ret(!0);
    while (!stopping() || !impl_->socks_.empty()) {

        if (detail::engineimpl::STOPPED == impl_->state_)
            break;

        try {

            if (impl_->timers_.empty()) {

                scoped_unblock unblock;
                while (AUG_RETINTR == (ret = waitnbevents
                                       (impl_->nbfiles_)))
                    ;

            } else {

                AUG_DEBUG2("processing timers");

                timeval tv;
                foreachexpired(impl_->timers_, 0 == ret, tv);

                scoped_unblock unblock;
                while (AUG_RETINTR == (ret = waitnbevents
                                       (impl_->nbfiles_, tv)))
                    ;
            }

            // Notify of any established connections before processing the
            // files: data may have arrived on a newly established connection.

            while (!impl_->pending_.empty()) {
                setconnected(*impl_->pending_.front());
                impl_->pending_.pop();
            }

            AUG_DEBUG2("processing files");

            foreachnbfile(impl_->nbfiles_);
            continue;

        } AUG_PERRINFOCATCH;

        // When running in foreground, stop on error.

        if (stoponerr)
            teardown();
    }
}

AUGRTPP_API void
engine::teardown()
{
    impl_->socks_.teardown();
}

AUGRTPP_API void
engine::reconfall()
{
    aug_event e = { AUG_EVENTRECONF, AUG_VARNULL };
    writeevent(impl_->eventwr_, e);
}

AUGRTPP_API void
engine::stopall()
{
    aug_event e = { AUG_EVENTSTOP, AUG_VARNULL };
    writeevent(impl_->eventwr_, e);
}

AUGRTPP_API void
engine::post(const char* sname, const char* to, const char* type,
             const aug_var* var)
{
    auto_ptr<postevent> arg(new postevent(sname, to, type));
    aug_event e;
    e.type_ = POSTEVENT_;
    e.var_.type_ = 0;
    e.var_.arg_ = arg.get();
    writeevent(impl_->eventwr_, e);

    // Event takes ownership of var when post cannot fail.

    aug_setvar(&arg->var_, var);
    arg.release();
}

AUGRTPP_API void
engine::dispatch(const char* sname, const char* to, const char* type,
                 const void* user, size_t size)
{
    vector<sessionptr> sessions;
    impl_->sessions_.getbygroup(sessions, to);

    vector<sessionptr>::const_iterator it(sessions.begin()),
        end(sessions.end());
    for (; it != end; ++it)
        (*it)->event(sname, type, user, size);
}

AUGRTPP_API void
engine::shutdown(augrt_id cid)
{
    sockptr sock(impl_->socks_.getbyid(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    if (null != cptr)
        cptr->shutdown();
    else
        impl_->socks_.erase(*sock);
}

AUGRTPP_API augrt_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   void* user)
{
    sessionptr session(impl_->sessions_.getbyname(sname));
    connptr cptr(new clntconn(session, user, impl_->timers_, host, port));

    // Remove on exception.

    scoped_insert si(impl_->socks_, cptr);

    if (CONNECTED == cptr->state()) {

        // connected() must only be called after this function has returned.

        insertnbfile(impl_->nbfiles_, cptr->sfd(), *impl_);
        setnbeventmask(cptr->sfd(), AUG_FDEVENTRD);

        if (impl_->pending_.empty()) {

            // Schedule an event to ensure that connected() is called after
            // this function has returned.

            aug_event e = { AUG_EVENTWAKEUP, AUG_VARNULL };
            writeevent(impl_->eventwr_, e);
        }

        // Add to pending queue.

        impl_->pending_.push(cptr);

    } else {

        insertnbfile(impl_->nbfiles_, cptr->sfd(),  *impl_);
        setnbeventmask(cptr->sfd(), AUG_FDEVENTALL);
    }

    si.commit();
    return id(*cptr);
}

AUGRTPP_API augrt_id
engine::tcplisten(const char* sname, const char* host, const char* port,
                  void* user)
{
    // Bind listener socket.

    endpoint ep(null);
    smartfd sfd(aug::tcplisten(host, port, ep));

    insertnbfile(impl_->nbfiles_, sfd, *impl_);
    setnbeventmask(sfd, AUG_FDEVENTRD);

    inetaddr addr(null);
    AUG_DEBUG2("listening: interface=[%s], port=[%d]",
               inetntop(getinetaddr(ep, addr)).c_str(),
               static_cast<int>(ntohs(aug::port(ep))));

    // Prepare state.

    sessionptr session(impl_->sessions_.getbyname(sname));
    listenerptr lptr(new listener(session, user, sfd));
    scoped_insert si(impl_->socks_, lptr);

    si.commit();
    return id(*lptr);
}

AUGRTPP_API void
engine::send(augrt_id cid, const void* buf, size_t len)
{
    if (!impl_->socks_.send(cid, buf, len))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

AUGRTPP_API void
engine::sendv(augrt_id cid, const aug_var& var)
{
    if (!impl_->socks_.sendv(cid, var))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

AUGRTPP_API void
engine::setrwtimer(augrt_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_EEXIST,
                          "connection not found: id=[%d]", cid);
    rwtimer->setrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::resetrwtimer(augrt_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    return rwtimer->resetrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::cancelrwtimer(augrt_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);
    return rwtimer->cancelrwtimer(flags);
}

AUGRTPP_API augrt_id
engine::settimer(const char* sname, unsigned ms, const aug_var* var)
{
    augrt_id id(aug_nextid());
    aug_var local = { 0, impl_ };

    // If aug_settimer() succeeds, aug_destroyvar() will be called on var when
    // the timer is destroyed.

    aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                  &detail::engineimpl::timercb>, local);

    sessiontimerptr tptr(new sessiontimer(impl_->sessions_.getbyname(sname)));
    impl_->sessiontimers_[id] = tptr;

    // Timer takes ownership of var when settimer cannot fail.

    aug_setvar(&tptr->var_, var);
    return id;
}

AUGRTPP_API bool
engine::resettimer(augrt_id tid, unsigned ms)
{
    return aug::resettimer(impl_->timers_, tid, ms);
}

AUGRTPP_API bool
engine::canceltimer(augrt_id tid)
{
    bool ret(aug::canceltimer(impl_->timers_, tid));

    // Only erase if aug_canceltimer() returns true: it may be in the midst of
    // a aug_foreachexpired() call, in which case, aug_canceltimer() will
    // return false for the timer being expired.

    if (ret)
        impl_->sessiontimers_.erase(tid);
    return ret;
}

AUGRTPP_API void
engine::setsslclient(augrt_id cid, sslctx& ctx)
{
#if ENABLE_SSL
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    aug::setsslclient(*cptr, ctx);
#else // !ENABLE_SSL
    throw local_error(__FILE__, __LINE__, AUG_ESUPPORT,
                      AUG_MSG("aug_setsslclient() not supported"));
#endif // !ENABLE_SSL
}

AUGRTPP_API void
engine::setsslserver(augrt_id cid, sslctx& ctx)
{
#if ENABLE_SSL
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    aug::setsslserver(*cptr, ctx);
#else // !ENABLE_SSL
    throw local_error(__FILE__, __LINE__, AUG_ESUPPORT,
                      AUG_MSG("aug_setsslserver() not supported"));
#endif // !ENABLE_SSL
}

AUGRTPP_API bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
