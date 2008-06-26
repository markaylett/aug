/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/engine.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augaspp/clntconn.hpp"
#include "augaspp/listener.hpp"
#include "augaspp/servconn.hpp"
#include "augaspp/sessions.hpp"
#include "augaspp/socks.hpp"
#include "augaspp/ssl.hpp"

#include "augsrvpp/signal.hpp"

#include "augutilpp/timer.hpp"

#include "augext/msg.h"

#include <map>
#include <queue>

#define POSTEVENT_ (AUG_EVENTUSER - 1)

using namespace aug;
using namespace aug;
using namespace std;

namespace {

    class msgevent : public ref_base {
        msg<msgevent> msg_;
        const string from_, to_, type_;
        smartob<aug_object> payload_;
        msgevent(const string& from, const string& to, const string& type)
            : from_(from),
              to_(to),
              type_(type),
              payload_(null)
        {
            msg_.reset(this);
        }
    protected:
        ~msgevent() AUG_NOTHROW
        {
            // Deleted from base.
        }
    public:
        smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (equalid<aug_object>(id) || equalid<aug_msg>(id))
                return object_retain<aug_object>(msg_);
            return null;
        }
        void
        setpayload_(objectref payload) AUG_NOTHROW
        {
            payload_ = object_retain<aug_object>(payload);
        }
        const char*
        getfrom_() AUG_NOTHROW
        {
            return from_.c_str();
        }
        const char*
        getto_() AUG_NOTHROW
        {
            return to_.c_str();
        }
        const char*
        gettype_() AUG_NOTHROW
        {
            return type_.c_str();
        }
        smartob<aug_object>
        getpayload_() AUG_NOTHROW
        {
            return payload_;
        }
        static smartob<aug_msg>
        create(const string& from, const string& to, const string& type)
        {
            msgevent* ptr = new msgevent(from, to, type);
            return object_attach<aug_msg>(ptr->msg_);
        }
    };

    typedef std::queue<connptr> pending;

    void
    setconnected(conn_base& conn, const timeval& now)
    {
        // Connection has now been established.

        //setsockopts(conn.sd());

        string name(conn.peername());
        inetaddr addr(null);
        AUG_CTXDEBUG2(aug_tlx, "connected: name=[%s]", name.c_str());

        //setnbeventmask(conn.sd(), AUG_FDEVENTRD);

        // Notify session of establishment.

        conn.connected(name, now);
    }
}

AUGRTPP_API
enginecb_base::~enginecb_base() AUG_NOTHROW
{
}

namespace aug {

    namespace detail {

        // Definition placed outside anonymous namespace to avoid compiler
        // warnings.

        struct sessiontimer {
            sessionptr session_;
            smartob<aug_object> ob_;
            sessiontimer(const sessionptr& session,
                         const smartob<aug_object>& ob)
                : session_(session),
                  ob_(ob)
            {
            }
        };

        typedef map<mod_id, sessiontimer> sessiontimers;

        struct engineimpl {

            chandler<engineimpl> chandler_;
            mdref eventrd_, eventwr_;
            timers& timers_;
            enginecb_base& cb_;
            muxer muxer_;
            chans chans_;
            sessions sessions_;
            socks socks_;

            // Grace period on shutdown.

            timer grace_;

            // Mapping of timer-ids to sessions.

            sessiontimers sessiontimers_;

            // Pending calls to connected().

            pending pending_;

            timeval now_;

            enum {
                STARTED,
                TEARDOWN,
                STOPPED
            } state_;

            engineimpl(mdref eventrd, mdref eventwr, timers& timers,
                       enginecb_base& cb)
                : eventrd_(eventrd),
                  eventwr_(eventwr),
                  timers_(timers),
                  cb_(cb),
                  chans_(null),
                  grace_(timers_),
                  state_(STARTED)
            {
                chandler_.reset(this);
                chans tmp(getmpool(aug_tlx), chandler_);
                chans_.swap(tmp);

                gettimeofday(now_);
                setfdeventmask(muxer_, eventrd_, AUG_FDEVENTRD);
            }
            smartob<aug_object>
            cast_(const char* id) AUG_NOTHROW
            {
                if (equalid<aug_object>(id) || equalid<aug_chandler>(id))
                    return object_retain<aug_object>(chandler_);
                return null;
            }
            void
            retain_() AUG_NOTHROW
            {
            }
            void
            release_() AUG_NOTHROW
            {
            }
            void
            clearchan_(unsigned id) AUG_NOTHROW
            {
            }
            aug_bool
            estabchan_(unsigned id, obref<aug_stream> stream,
                       unsigned parent) AUG_NOTHROW
            {
                return AUG_TRUE;
            }
            aug_bool
            readychan_(unsigned id, obref<aug_stream> stream,
                       unsigned short events) AUG_NOTHROW
            {
                sockptr sock(socks_.getbyid(id));
                connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

                AUG_CTXDEBUG2(aug_tlx, "processing sock: id=[%u]", id);

                if (0 == events) {

                    if (null == ptr)
                        return accept();

                    return AUG_TRUE;
                }
                //-

                bool changed = false, ok = false;
                try {
                    changed = cptr->process(events, now_);
                    ok = true;
                } AUG_PERRINFOCATCH;

                // If an exception was thrown, "ok" will still have its
                // original value of false.

                if (!ok) {

                    // Connection is closed if an exception is thrown during
                    // processing.

                    socks_.erase(*cptr);
                    return AUG_FALSE;
                }

                if (HANDSHAKE == cptr->state()) {

                    // The associated file descriptor may change as connection
                    // attempts fail and alternative addresses are tried.

                    insertnbfile(nbfiles_, cptr->sd(), *this);
                    setnbeventmask(cptr->sd(), AUG_FDEVENTALL);

                    socks_.update(cptr, md);

                } else if (changed)

                    switch (cptr->state()) {
                    case CONNECTED:

                        // Was connecting, now established: notify module of
                        // connection establishment.

                        setconnected(*cptr, now_);
                        break;
                    case CLOSED:
                        socks_.erase(*cptr);
                        return AUG_FALSE;
                    default:
                        break;
                    }

                return AUG_TRUE;
            }
            void
            teardown()
            {
                if (STARTED == state_) {

                    state_ = TEARDOWN;
                    socks_.teardown(now_);

                    // Initiate grace period.

                    smartob<aug_boxptr> ob(createboxptr(this, 0));
                    grace_.set(15000, timermemcb<engineimpl,
                               &engineimpl::stopcb>, ob);
                }
            }
            bool
            accept(const sock_base& sock)
            {
                AUG_CTXDEBUG2(aug_tlx, "accepting connection");

                connptr cptr(new servconn(sock.session(), user(sock),
                                          timers_, sd, ep));

                scoped_insert si(socks_, cptr);
                AUG_CTXDEBUG2(aug_tlx,
                              "initialising connection: id=[%d], fd=[%d]",
                              id(*cptr), sd.get());

                // Session may reject the connection by returning false.

                if (!cptr->accepted(ep, now_))
                    return false;

                si.commit();
                return true;
            }
            bool
            readevent()
            {
                AUG_CTXDEBUG2(aug_tlx, "reading event");

                pair<int, smartob<aug_object> >
                    event(aug::readevent(eventrd_));

                switch (event.first) {
                case AUG_EVENTRECONF:
                    AUG_CTXDEBUG2(aug_tlx, "received AUG_EVENTRECONF");
                    cb_.reconf();
                    sessions_.reconf();
                    break;
                case AUG_EVENTSTATUS:
                    AUG_CTXDEBUG2(aug_tlx, "received AUG_EVENTSTATUS");
                    break;
                case AUG_EVENTSTOP:
                    AUG_CTXDEBUG2(aug_tlx, "received AUG_EVENTSTOP");
                    teardown();
                    break;
                case AUG_EVENTSIGNAL:
                    AUG_CTXDEBUG2(aug_tlx, "received AUG_EVENTSIGNAL");
                    break;
                case AUG_EVENTWAKEUP:
                    AUG_CTXDEBUG2(aug_tlx, "received AUG_EVENTWAKEUP");
                    // Actual handling is performed in do_run().
                    break;
                case POSTEVENT_:
                    AUG_CTXDEBUG2(aug_tlx, "received POSTEVENT_");
                    {
                        smartob<aug_eventob> ev
                            (object_cast<aug_eventob>(event.second));

                        vector<sessionptr> sessions;
                        sessions_.getbygroup(sessions, eventobto(ev));

                        smartob<aug_object> user(eventobuser(ev));
                        vector<sessionptr>
                            ::const_iterator it(sessions.begin()),
                            end(sessions.end());
                        for (; it != end; ++it)
                            (*it)->event(eventobfrom(ev), eventobtype(ev),
                                         user);
                    }
                }
                return true;
            }
            bool
            nbfilecb(mdref md, unsigned short events)
            {
                sockptr sock(socks_.getbysd(md));
                connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

                AUG_CTXDEBUG2(aug_tlx, "processing sock: id=[%d], fd=[%d]",
                              id(*sock), md.get());

                if (null != cptr)
                    return process(cptr, md, events);

                accept(*sock);
                return true;
            }
            void
            timercb(idref id, unsigned& ms)
            {
                AUG_CTXDEBUG2(aug_tlx, "custom timer expiry");

                sessiontimers::iterator it(sessiontimers_.find(id.get()));
                mod_handle timer = { id.get(), it->second.ob_.get() };
                it->second.session_->expire(timer, ms);

                if (0 == ms) {
                    AUG_CTXDEBUG2(aug_tlx,
                                  "removing timer: ms has been set to zero");
                    sessiontimers_.erase(it);
                }
            }
            void
            stopcb(idref id, unsigned& ms)
            {
                // Called by grace timer when excessive time has been spent in
                // teardown state.

                aug_ctxinfo(aug_tlx, "giving-up, closing connections");
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
engine::engine(mdref eventrd, mdref eventwr, timers& timers,
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

    detail::sessiontimers::iterator it(impl_->sessiontimers_.begin()),
        end(impl_->sessiontimers_.end());
    while (it != end) {
        if (!it->second.session_->active()) {
            aug_ctxwarn(aug_tlx,
                        "cancelling timer associated with inactive session");
            aug_canceltimer(cptr(impl_->timers_), it->first);
            impl_->sessiontimers_.erase(it++);
        } else
            ++it;
    }
}

AUGRTPP_API void
engine::run(bool stoponerr)
{
    AUG_CTXDEBUG2(aug_tlx, "running daemon process");

    int ret(!0);
    while (!stopping() || !impl_->socks_.empty()) {

        if (detail::engineimpl::STOPPED == impl_->state_)
            break;

        try {

            if (impl_->timers_.empty()) {

                scoped_unblock unblock;
                while (AUG_FAILINTR == (ret = waitnbevents
                                        (impl_->nbfiles_)))
                    ;

            } else {

                AUG_CTXDEBUG2(aug_tlx, "processing timers");

                timeval tv;
                foreachexpired(impl_->timers_, 0 == ret, tv);

                scoped_unblock unblock;
                while (AUG_FAILINTR == (ret = waitnbevents
                                        (impl_->nbfiles_, tv)))
                    ;
            }

            // Update timestamp after waiting.

            gettimeofday(impl_->now_);

            // Notify of any established connections before processing the
            // files: data may have arrived on a newly established connection.

            while (!impl_->pending_.empty()) {
                setconnected(*impl_->pending_.front(), impl_->now_);
                impl_->pending_.pop();
            }

            AUG_CTXDEBUG2(aug_tlx, "processing events");

            if (fdevents(muxer_, rd_))
                readevent();

            AUG_CTXDEBUG2(aug_tlx, "processing files");

            foreachnbfile(impl_->nbfiles_);

            continue;

        } AUG_PERRINFOCATCH;

        // When running in foreground, stop on error.

        if (stoponerr)
            impl_->socks_.teardown(impl_->now_);
    }
}

AUGRTPP_API void
engine::reconfall()
{
    aug_event e = { AUG_EVENTRECONF, 0 };
    writeevent(impl_->eventwr_, e);
}

AUGRTPP_API void
engine::stopall()
{
    aug_event e = { AUG_EVENTSTOP, 0 };
    writeevent(impl_->eventwr_, e);
}

AUGRTPP_API void
engine::post(const char* sname, const char* to, const char* type,
             objectref ob)
{
    smartob<aug_eventob> ev(postevent::create(sname, to, type));
    seteventobuser(ev, ob);
    aug_event e;
    e.type_ = POSTEVENT_;
    e.ob_ = ev.base();

    writeevent(impl_->eventwr_, e);
}

AUGRTPP_API void
engine::dispatch(const char* sname, const char* to, const char* type,
                 objectref ob)
{
    vector<sessionptr> sessions;
    impl_->sessions_.getbygroup(sessions, to);

    vector<sessionptr>::const_iterator it(sessions.begin()),
        end(sessions.end());
    for (; it != end; ++it)
        (*it)->event(sname, type, ob);
}

AUGRTPP_API void
engine::shutdown(mod_id cid, unsigned flags)
{
    sockptr sock(impl_->socks_.getbyid(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    if (null != cptr) {
        cptr->shutdown(flags, impl_->now_);

        // Forced shutdown: may be used on misbehaving clients.

        if (flags & MOD_SHUTNOW)
            impl_->socks_.erase(*sock);
    } else
        impl_->socks_.erase(*sock);
}

AUGRTPP_API mod_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   void* user)
{
    sessionptr session(impl_->sessions_.getbyname(sname));
    connptr cptr(new clntconn(session, user, impl_->timers_, host, port));

    // Remove on exception.

    scoped_insert si(impl_->socks_, cptr);

    if (CONNECTED == cptr->state()) {

        // connected() must only be called after this function has returned.

        insertnbfile(impl_->nbfiles_, cptr->sd(), *impl_);
        setnbeventmask(cptr->sd(), AUG_FDEVENTRD);

        if (impl_->pending_.empty()) {

            // Schedule an event to ensure that connected() is called after
            // this function has returned.

            aug_event e = { AUG_EVENTWAKEUP, 0 };
            writeevent(impl_->eventwr_, e);
        }

        // Add to pending queue.

        impl_->pending_.push(cptr);

    } else {

        insertnbfile(impl_->nbfiles_, cptr->sd(),  *impl_);
        setnbeventmask(cptr->sd(), AUG_FDEVENTALL);
    }

    si.commit();
    return id(*cptr);
}

AUGRTPP_API mod_id
engine::tcplisten(const char* sname, const char* host, const char* port,
                  void* user)
{
    // Bind listener socket.

    endpoint ep(null);
    autosd sd(aug::tcplisten(host, port, ep));

    insertnbfile(impl_->nbfiles_, sd, *impl_);
    setnbeventmask(sd, AUG_FDEVENTRD);

    inetaddr addr(null);
    AUG_CTXDEBUG2(aug_tlx, "listening: interface=[%s], port=[%d]",
                  inetntop(getinetaddr(ep, addr)).c_str(),
                  static_cast<int>(ntohs(aug::port(ep))));

    // Prepare state.

    sessionptr session(impl_->sessions_.getbyname(sname));
    listenerptr lptr(new listener(session, user, sd));
    scoped_insert si(impl_->socks_, lptr);

    si.commit();
    return id(*lptr);
}

AUGRTPP_API void
engine::send(mod_id cid, const void* buf, size_t len)
{
    if (!impl_->socks_.send(cid, buf, len, impl_->now_))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");
}

AUGRTPP_API void
engine::sendv(mod_id cid, blobref blob)
{
    if (!impl_->socks_.sendv(cid, blob, impl_->now_))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");
}

AUGRTPP_API void
engine::setrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_EEXIST,
                        "connection not found: id=[%d]", cid);
    rwtimer->setrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::resetrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]", cid);

    return rwtimer->resetrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::cancelrwtimer(mod_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]", cid);
    return rwtimer->cancelrwtimer(flags);
}

AUGRTPP_API mod_id
engine::settimer(const char* sname, unsigned ms, objectref ob)
{
    mod_id id(aug_nextid());
    smartob<aug_boxptr> local(createboxptr(impl_, 0));

    aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                  &detail::engineimpl::timercb>, local);

    // Insert after settimer() has succeeded.

    detail::sessiontimer timer(impl_->sessions_.getbyname(sname),
                               smartob<aug_object>::retain(ob));
    impl_->sessiontimers_.insert(make_pair(id, timer));
    return id;
}

AUGRTPP_API bool
engine::resettimer(mod_id tid, unsigned ms)
{
    return aug::resettimer(impl_->timers_, tid, ms);
}

AUGRTPP_API bool
engine::canceltimer(mod_id tid)
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
engine::setsslclient(mod_id cid, sslctx& ctx)
{
#if ENABLE_SSL
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]", cid);

    aug::setsslclient(*cptr, ctx);
#else // !ENABLE_SSL
    throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                    AUG_MSG("aug_setsslclient() not supported"));
#endif // !ENABLE_SSL
}

AUGRTPP_API void
engine::setsslserver(mod_id cid, sslctx& ctx)
{
#if ENABLE_SSL
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]", cid);

    aug::setsslserver(*cptr, ctx);
#else // !ENABLE_SSL
    throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                    AUG_MSG("aug_setsslserver() not supported"));
#endif // !ENABLE_SSL
}

AUGRTPP_API bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
