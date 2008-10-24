/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
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
        setmsgpayload_(objectref payload) AUG_NOTHROW
        {
            payload_ = object_retain<aug_object>(payload);
        }
        const char*
        getmsgfrom_() AUG_NOTHROW
        {
            return from_.c_str();
        }
        const char*
        getmsgto_() AUG_NOTHROW
        {
            return to_.c_str();
        }
        const char*
        getmsgtype_() AUG_NOTHROW
        {
            return type_.c_str();
        }
        smartob<aug_object>
        getmsgpayload_() AUG_NOTHROW
        {
            return payload_;
        }
        static smartob<aug_msg>
        create(const string& from, const string& to, const string& type)
        {
            // Events can be posted between threads.  The event object is
            // allocated on the freestore to avoid use of aug_tlx.

            msgevent* ptr = new msgevent(from, to, type);
            return object_attach<aug_msg>(ptr->msg_);
        }
    };
}

AUGASPP_API
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

        struct engineimpl : mpool_base {

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
                  muxer_(getmpool(aug_tlx)),
                  chans_(null),
                  grace_(timers_),
                  state_(STARTED)
            {
                chandler_.reset(this);
                chans tmp(getmpool(aug_tlx), chandler_);
                chans_.swap(tmp);

                gettimeofday(now_);
                setmdeventmask(muxer_, eventrd_, AUG_MDEVENTRD);
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
                // FIXME: listener will not exist after teardown.

                socks_.erase(id);
            }
            void
            errorchan_(unsigned id, const aug_errinfo& errinfo) AUG_NOTHROW
            {
                // FIXME: implement.
            }
            aug_bool
            estabchan_(unsigned id, obref<aug_stream> stream,
                       unsigned parent) AUG_NOTHROW
            {
                chanptr chan(object_cast<aug_chan>(stream));

                if (id == parent) {

                    // Was connecting, now established: notify module of
                    // connection establishment.

                    sockptr sock(socks_.get(id));
                    connptr conn(smartptr_cast<conn_base>(sock)); // Downcast.

                    // Connection has now been established.

                    string name(conn->peername(chan));
                    AUG_CTXDEBUG2(aug_tlx, "connected: name=[%s]",
                                  name.c_str());

                    // Notify session of establishment.

                    conn->connected(name, now_);
                    return AUG_TRUE;
                }

                AUG_CTXDEBUG2(aug_tlx, "accepting connection");

                sockptr sock(socks_.get(parent));
                connptr conn(new servconn(getmpool(aug_tlx), sock->session(),
                                          user(*sock), timers_,
                                          getchanid(chan)));
                scoped_insert si(socks_, conn);

                // Connection has now been established.

                string name(conn->peername(chan));
                AUG_CTXDEBUG2(aug_tlx,
                              "initialising connection: id=[%u], name=[%s]",
                              aug::id(*conn), name.c_str());

                // Session may reject the connection by returning false.

                if (!conn->accepted(name, now_))
                    return AUG_FALSE;

                si.commit();
                return AUG_TRUE;
            }
            aug_bool
            readychan_(unsigned id, obref<aug_stream> stream,
                       unsigned short events) AUG_NOTHROW
            {
                sockptr sock(socks_.get(id));
                connptr cptr(smartptr_cast<conn_base>(sock)); // Downcast.

                AUG_CTXDEBUG2(aug_tlx,
                              "processing sock: id=[%u], events=[%u]",
                              id, (unsigned)events);

                bool changed = false, threw = true;
                try {
                    changed = cptr->process(stream, events, now_);
                    threw = false;
                } catch (const block_exception&) {

                    AUG_CTXDEBUG2(aug_tlx, "operation would block: id=[%u]",
                                  id);

                    // FIXME: shutdown may have removed socket.

                    return socks_.exists(id)
                        ? AUG_TRUE : AUG_FALSE; // EWOULDBLOCK.

                } AUG_PERRINFOCATCH;

                // If an exception was thrown, "ok" will still have its
                // original value of false.

                if (threw) {

                    // Connection is closed if an exception is thrown during
                    // processing.

                    return AUG_FALSE;
                }

                if (changed && CLOSED == cptr->state())
                    return AUG_FALSE;

                // FIXME: shutdown may have removed socket.

                return socks_.exists(id) ? AUG_TRUE : AUG_FALSE;
            }
            void
            teardown()
            {
                if (STARTED == state_) {

                    state_ = TEARDOWN;
                    socks_.teardown(now_);

                    // Allow grace period before forcefully stopping the
                    // application.

                    smartob<aug_boxptr> ob
                        (createboxptr(aug_getmpool(aug_tlx), this, 0));
                    grace_.set(15000, timermemcb<engineimpl,
                               &engineimpl::stopcb>, ob);
                }
            }
            bool
            readevent()
            {
                AUG_CTXDEBUG2(aug_tlx, "reading event");

                // Sticky events not required for fixed length blocking read.

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
                        smartob<aug_msg> msg
                            (object_cast<aug_msg>(event.second));

                        vector<sessionptr> sessions;
                        sessions_.getbygroup(sessions, getmsgto(msg));

                        smartob<aug_object> payload(getmsgpayload(msg));
                        vector<sessionptr>
                            ::const_iterator it(sessions.begin()),
                            end(sessions.end());
                        for (; it != end; ++it)
                            (*it)->event(getmsgfrom(msg), getmsgtype(msg),
                                         payload);
                    }
                }
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

AUGASPP_API
engine::~engine() AUG_NOTHROW
{
    delete impl_;
}

AUGASPP_API
engine::engine(mdref eventrd, mdref eventwr, timers& timers,
               enginecb_base& cb)
    : impl_(new detail::engineimpl(eventrd, eventwr, timers, cb))
{
}

AUGASPP_API void
engine::clear()
{
    {
        chans tmp(null);
        impl_->chans_.swap(tmp);
    }

    // FIXME: assert that socks are already clear?

    impl_->socks_.clear();

    // FIXME: erase the sessions in reverse order to which they were added.

    impl_->sessions_.clear();
}

AUGASPP_API void
engine::insert(const string& name, const sessionptr& session,
               const char* groups)
{
    impl_->sessions_.insert(name, session, groups);
}

AUGASPP_API void
engine::cancelinactive()
{
    // Remove any timers allocated to sessions that could not be opened.

    detail::sessiontimers::iterator it(impl_->sessiontimers_.begin()),
        end(impl_->sessiontimers_.end());
    while (it != end) {
        if (!it->second.session_->active()) {
            aug_ctxwarn(aug_tlx,
                        "cancelling timer associated with inactive session");
            aug_canceltimer(impl_->timers_, it->first);
            impl_->sessiontimers_.erase(it++);
        } else
            ++it;
    }
}

AUGASPP_API void
engine::run(bool stoponerr)
{
    AUG_CTXDEBUG2(aug_tlx, "running daemon process");
    unsigned ready(!0);
    while (!(stopping() && impl_->socks_.empty())) {

        if (detail::engineimpl::STOPPED == impl_->state_) {
            // Forcefully stopped.
            break;
        }

        try {

            AUG_CTXDEBUG2(aug_tlx, "processing timers");

            timeval tv;
            processexpired(impl_->timers_, 0 == ready, tv);

            try {

                scoped_unblock unblock;

                ready = getreadychans(impl_->chans_);
                if (ready) {

                    // Some are ready so don't block.

                    pollmdevents(impl_->muxer_);

                } else if (impl_->timers_.empty()) {

                    // No timers so wait indefinitely.

                    ready = waitmdevents(impl_->muxer_);

                } else {

                    // Wait upto next timer expiry.

                    ready = waitmdevents(impl_->muxer_, tv);
                }

            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            // Update timestamp after waiting.

            gettimeofday(impl_->now_);

            AUG_CTXDEBUG2(aug_tlx, "processing events");

            // Sticky events not required for fixed length blocking read.

            if (getmdevents(impl_->muxer_, impl_->eventrd_))
                impl_->readevent();

            AUG_CTXDEBUG2(aug_tlx, "processing files");

            processchans(impl_->chans_);
            continue;

        } AUG_PERRINFOCATCH;

        // When running in foreground, stop on error.

        if (stoponerr)
            impl_->socks_.teardown(impl_->now_);
    }
}

AUGASPP_API void
engine::reconfall()
{
    // Thread-safe.

    aug_event e = { AUG_EVENTRECONF, 0 };
    writeevent(impl_->eventwr_, e);
}

AUGASPP_API void
engine::stopall()
{
    // Thread-safe.

    aug_event e = { AUG_EVENTSTOP, 0 };
    writeevent(impl_->eventwr_, e);
}

AUGASPP_API void
engine::post(const char* sname, const char* to, const char* type,
             objectref ob)
{
    // Thread-safe.

    smartob<aug_msg> msg(msgevent::create(sname, to, type));
    setmsgpayload(msg, ob);
    aug_event e;
    e.type_ = POSTEVENT_;
    e.ob_ = msg.base();

    writeevent(impl_->eventwr_, e);
}

AUGASPP_API void
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

AUGASPP_API void
engine::shutdown(mod_id cid, unsigned flags)
{
    sockptr sock(impl_->socks_.get(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    chanptr chan(findchan(impl_->chans_, cid));
    if (null != cptr) {
        cptr->shutdown(chan, flags, impl_->now_);

        // Forced shutdown: may be used on misbehaving clients.

        if (flags & MOD_SHUTNOW)
            impl_->socks_.erase(*sock);
    } else
        impl_->socks_.erase(*sock);
}

AUGASPP_API mod_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   sslctx* ctx, void* user)
{
#if !ENABLE_SSL
    if (ctx)
        throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                        AUG_MSG("ssl not supported"));
#endif // !ENABLE_SSL

    sessionptr session(impl_->sessions_.getbyname(sname));

    mpoolptr mpool(getmpool(aug_tlx));
    chanptr chan(createclient(mpool, host, port, impl_->muxer_,
                              ctx ? ctx->get() : 0));
    connptr conn(new clntconn(mpool, session, user, impl_->timers_,
                              getchanid(chan)));

    impl_->socks_.insert(conn);
    insertchan(impl_->chans_, chan);
    return id(*conn);
}

AUGASPP_API mod_id
engine::tcplisten(const char* sname, const char* host, const char* port,
                  sslctx* ctx, void* user)
{
#if !ENABLE_SSL
    if (ctx)
        throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                        AUG_MSG("ssl not supported"));
#endif // !ENABLE_SSL

    // Bind listener socket.

    endpoint ep(null);
    autosd sd(tcpserver(host, port, ep));
    setnonblock(sd, true);

    chanptr chan(createserver(getmpool(aug_tlx), impl_->muxer_, sd,
                              ctx ? ctx->get() : 0));
    sd.release();

    inetaddr addr(null);
    AUG_CTXDEBUG2(aug_tlx, "listening: interface=[%s], port=[%d]",
                  inetntop(getinetaddr(ep, addr)).c_str(),
                  static_cast<int>(ntohs(aug::port(ep))));

    // Prepare state.

    sessionptr session(impl_->sessions_.getbyname(sname));
    listenerptr conn(new listener(session, user, chan));

    impl_->socks_.insert(conn);
    insertchan(impl_->chans_, chan);
    return id(*conn);
}

AUGASPP_API void
engine::send(mod_id cid, const void* buf, size_t len)
{
    sockptr sock(impl_->socks_.get(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    chanptr chan(findchan(impl_->chans_, cid));

    if (null == cptr || !sendable(*cptr))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");

    cptr->send(chan, buf, len, impl_->now_);
}

AUGASPP_API void
engine::sendv(mod_id cid, blobref blob)
{
    sockptr sock(impl_->socks_.get(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    chanptr chan(findchan(impl_->chans_, cid));

    if (null == cptr || !sendable(*cptr))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");

    cptr->sendv(chan, blob, impl_->now_);
}

AUGASPP_API void
engine::setrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_EEXIST,
                        "connection not found: id=[%u]", cid);
    rwtimer->setrwtimer(ms, flags);
}

AUGASPP_API bool
engine::resetrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%u]", cid);

    return rwtimer->resetrwtimer(ms, flags);
}

AUGASPP_API bool
engine::cancelrwtimer(mod_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%u]", cid);
    return rwtimer->cancelrwtimer(flags);
}

AUGASPP_API mod_id
engine::settimer(const char* sname, unsigned ms, objectref ob)
{
    mod_id id(aug_nextid());
    smartob<aug_boxptr> local(createboxptr(getmpool(aug_tlx), impl_, 0));

    aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                  &detail::engineimpl::timercb>, local);

    // Insert after settimer() has succeeded.

    detail::sessiontimer timer(impl_->sessions_.getbyname(sname),
                               smartob<aug_object>::retain(ob));
    impl_->sessiontimers_.insert(make_pair(id, timer));
    return id;
}

AUGASPP_API bool
engine::resettimer(mod_id tid, unsigned ms)
{
    try {
        aug::resettimer(impl_->timers_, tid, ms);
    } catch (const none_exception&) {
        return false;
    }
    return true;
}

AUGASPP_API bool
engine::canceltimer(mod_id tid)
{
    try {

        aug::canceltimer(impl_->timers_, tid);

        // Only erase if aug_canceltimer() succeeded: it may be in the midst
        // of a aug_foreachexpired() call, in which case, aug_canceltimer()
        // will throw for the timer being expired.

        impl_->sessiontimers_.erase(tid);

    } catch (const none_exception&) {
        return false;
    }
    return true;
}

AUGASPP_API bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
