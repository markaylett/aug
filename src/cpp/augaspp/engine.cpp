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
#define AUGASPP_BUILD
#include "augaspp/engine.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augaspp/clntconn.hpp"
#include "augaspp/cluster.hpp"
#include "augaspp/listener.hpp"
#include "augaspp/servconn.hpp"
#include "augaspp/sessions.hpp"
#include "augaspp/socks.hpp"
#include "augaspp/ssl.hpp"
#include "augaspp/types.hpp"

#include "augservpp/signal.hpp"

#include "augutilpp/timer.hpp"

#include "augext/msg.h"

#include <map>
#include <queue>

#define POSTEVENT_ (AUG_EVENTUSER - 1)
#define LOGCLUSTER_ 0

using namespace aug;
using namespace std;

namespace {

    enum scheme {
        NONE,
        NODE,
        TOPIC
    };

    pair<scheme, string>
    splituri(const string& uri)
    {
        string x, y;
        scheme s(NONE);
        if (split2(uri.begin(), uri.end(), x, y, ':')) {
            if (x == "node")
                s = NODE;
            else if (x == "topic")
                s = TOPIC;
            else
                throw aug_error(__FILE__, __LINE__, AUG_EINVAL,
                                AUG_MSG("invalid scheme '%s'"), x.c_str());
        } else
            y = uri;
        return make_pair(s, y);
    }

    string
    getnode(const string& s)
    {
        pair<scheme, string> uri(splituri(s));
        switch (uri.first) {
        case NONE:
            uri.second = s;
        case NODE:
            break;
        default:
            throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                            AUG_MSG("scheme not supported"));
        }
        return uri.second;
    }

    string
    gettopic(const string& s)
    {
        pair<scheme, string> uri(splituri(s));
        switch (uri.first) {
        case NONE:
            uri.second = s;
        case TOPIC:
            break;
        default:
            throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                            AUG_MSG("scheme not supported"));
        }
        return uri.second;
    }

    // Must not use mpool_ops.

    class msgevent : public msg_base<msgevent> {
        aug_id id_;
        const string from_, to_, type_;
        objectptr payload_;
        msgevent(aug_id id, const string& from, const string& to,
                 const string& type)
            : id_(id),
              from_(from),
              to_(to),
              type_(type),
              payload_(null)
        {
        }
    public:
        ~msgevent() AUG_NOTHROW
        {
            // Deleted from base.
        }
        void
        setmsgpayload_(objectref payload) AUG_NOTHROW
        {
            payload_ = object_retain<aug_object>(payload);
        }
        unsigned
        getmsgid_() AUG_NOTHROW
        {
            return id_;
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
        static msgptr
        create(aug_id id, const string& from, const string& to,
               const string& type)
        {
            // Events can be posted between threads.  The event object is
            // allocated on the freestore to avoid use of aug_tlx.

            msgevent* ptr = new msgevent(id, from, to, type);
            return attach(ptr);
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

        struct sessiontimer : mpool_ops {
            sessionptr session_;
            objectptr ob_;
            sessiontimer(const sessionptr& session, objectref ob)
                : session_(session),
                  ob_(object_retain(ob))
            {
            }
        };

        typedef map<mod_id, sessiontimer> sessiontimers;

        class scoped_current {
            chanptr& lhs_;
        public:
            ~scoped_current()
            {
                lhs_ = null; // Release.
            }
            scoped_current(chanptr& lhs, chanref rhs)
                : lhs_(lhs)
            {
                lhs = object_retain(rhs);
            }
        };

        class packet : public mpool_ops {
            char node_[AUG_PKTNODELEN + 1];
            unsigned sess_;
            aug_seqno_t seqno_;
        public:
            explicit
            packet(const char* node)
            {
                aug_strlcpy(node_, node, sizeof(node_));
                sess_ = getpid();
                seqno_ = 0;
            }
            size_t
            emit(sdref ref, unsigned type, const void* data, unsigned size,
                 const endpoint& ep)
            {
                struct aug_packet pkt;
                size = AUG_MIN(size, sizeof(pkt.data_));
                aug_setpacket(node_, sess_, AUG_PKTUSER + type, ++seqno_,
                              data, size, &pkt);

                char buf[AUG_PACKETSIZE];
                aug_encodepacket(&pkt, buf);
                return aug::sendto(ref, buf, sizeof(buf), 0, ep);
            }
        };

        struct engineimpl : mpool_ops {

            chandler<engineimpl> chandler_;
            aug_muxer_t muxer_;
            aug_events_t events_;
            aug_timers_t timers_;
            enginecb_base& cb_;
            chans chans_;
            sessions sessions_;
            socks socks_;

            // Current channel.

            chanptr current_;

            // Grace period on shutdown.

            timer grace_;

            // Mapping of timer-ids to sessions.

            sessiontimers sessiontimers_;

            aug_timeval now_;

            enum {
                STARTED,
                TEARDOWN,
                STOPPED
            } state_;

#if ENABLE_MULTICAST
            types types_;
            packet mpacket_;
            autosd mcastsd_;
            endpoint mcastep_;
            cluster cluster_;
            timer mwait_;
#else // !ENABLE_MULTICAST
            const string node_;
#endif // !ENABLE_MULTICAST

            engineimpl(const char* node, aug_muxer_t muxer,
                       aug_events_t events, aug_timers_t timers,
                       enginecb_base& cb, unsigned timeout)
                : muxer_(muxer),
                  events_(events),
                  timers_(timers),
                  cb_(cb),
                  chans_(null),
                  current_(null),
                  grace_(timers_),
                  state_(STARTED),
#if ENABLE_MULTICAST
                  mpacket_(node),
                  mcastsd_(null),
                  mcastep_(null),
                  cluster_(getclock(aug_tlx), 8, timeout),
                  mwait_(timers, null)
#else // !ENABLE_MULTICAST
                  node_(node)
#endif // !ENABLE_MULTICAST
            {
                chandler_.reset(this);
                chans tmp(getmpool(aug_tlx), chandler_);
                chans_.swap(tmp);

                gettimeofday(getclock(aug_tlx), now_);
                setmdeventmask(muxer_, eventsmd(events_), AUG_MDEVENTRDEX);
            }
            objectptr
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
            aug_bool
            authchan_(aug_id id, const char* subject,
                      const char* issuer) AUG_NOTHROW
            {
                chanptr chan(aug::findchan(chans_, id));
                assert(null != chan);
                scoped_current current(current_, chan);

                sockptr sock(socks_.get(id));
                assert(null != sock);
                connptr conn(smartptr_cast<conn_base>(sock)); // Downcast.
                assert(null != conn);

                AUG_CTXDEBUG2(aug_tlx, "auth channel: id=[%d], subject=[%s],"
                              " issuer=[%s]", static_cast<int>(id), subject,
                              issuer);
                return conn->auth(subject, issuer) ? AUG_TRUE : AUG_FALSE;
            }
            void
            clearchan_(aug_id id) AUG_NOTHROW
            {
                AUG_CTXDEBUG2(aug_tlx, "clear channel: id=[%d]",
                              static_cast<int>(id));

                // FIXME: listener will not exist after teardown.

                socks_.erase(id);
            }
            void
            errorchan_(chanref chan, const aug_errinfo& errinfo) AUG_NOTHROW
            {
                scoped_current current(current_, chan);
                const aug_id id(getchanid(chan));

                sockptr sock(socks_.get(id));
                assert(null != sock);

                AUG_CTXDEBUG2(aug_tlx, "error channel: id=[%d], desc=[%s]",
                              static_cast<int>(id), errinfo.desc_);
                return sock->error(errinfo.desc_);
            }
            aug_bool
            estabchan_(chanref chan, aug_id parent) AUG_NOTHROW
            {
                scoped_current current(current_, chan);

                const aug_id id(getchanid(chan));
                AUG_CTXDEBUG2(aug_tlx, "established channel: id=[%d]",
                              static_cast<int>(id));

                if (id == parent) {

                    // Was connecting, now established: notify module of
                    // connection establishment.

                    sockptr sock(socks_.get(id));
                    assert(null != sock);
                    connptr conn(smartptr_cast<conn_base>(sock)); // Downcast.
                    assert(null != conn);

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
                connptr conn(new (tlx) servconn(getmpool(aug_tlx),
                                                sock->session(), timers_, id,
                                                sock->ob()));
                scoped_insert si(socks_, conn);

                // Connection has now been established.

                string name(conn->peername(chan));
                AUG_CTXDEBUG2(aug_tlx,
                              "initialising connection: id=[%d], name=[%s]",
                              static_cast<int>(conn->id()), name.c_str());

                // Session may reject the connection by returning false.

                if (!conn->accepted(name, now_))
                    return AUG_FALSE;

                si.commit();
                return AUG_TRUE;
            }
            aug_bool
            readychan_(chanref chan, unsigned short events) AUG_NOTHROW
            {
                scoped_current current(current_, chan);

                const aug_id id(getchanid(chan));
                AUG_CTXDEBUG2(aug_tlx, "readychannel: id=[%d]",
                              static_cast<int>(id));

                sockptr sock(socks_.get(id));
                assert(null != sock);
                connptr conn(smartptr_cast<conn_base>(sock)); // Downcast.
                assert(null != conn);

                AUG_CTXDEBUG2(aug_tlx,
                              "processing sock: id=[%d], events=[%u]",
                              static_cast<int>(id),
                              static_cast<unsigned>(events));

                bool threw = true;
                try {
                    conn->process(chan, events, now_);
                    threw = false;
                } AUG_PERRINFOCATCH;

                // If an exception was thrown, "threw" will still have its
                // original value of false.

                if (threw) {

                    // Connection is closed if an exception is thrown during
                    // processing.

                    return AUG_FALSE;
                }

                return CLOSED != conn->state();
            }
            chanptr
            findchan(aug_id id)
            {
                // Avoid lookup if current channel.

                if (null != current_ && id == getchanid(current_))
                    return current_;

                return aug::findchan(chans_, id);
            }
            void
            teardown()
            {
                if (STARTED == state_) {

                    state_ = TEARDOWN;
                    socks_.teardown(chans_, now_);

                    // Allow grace period before forcefully stopping the
                    // application.

                    boxptrptr ob(createboxptr(aug_getmpool(aug_tlx), this,
                                              0));
                    grace_.set(15000, timermemcb<engineimpl,
                               &engineimpl::stopcb>, ob);
                }
            }
            void
            readevent()
            {
                AUG_CTXDEBUG2(aug_tlx, "reading event");

                // Sticky events not required for fixed length blocking read.

                pair<int, objectptr> event(aug::readevent(events_));

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
                        msgptr msg(object_cast<aug_msg>(event.second));

                        // Add uri scheme to topic name.

                        string from("topic:");
                        from += getmsgfrom(msg);

                        vector<sessionptr> sessions;
                        sessions_.getbytopic(sessions, getmsgto(msg));

                        objectptr payload(getmsgpayload(msg));
                        vector<sessionptr>
                            ::const_iterator it(sessions.begin()),
                            end(sessions.end());
                        for (; it != end; ++it)
                            (*it)->event(from.c_str(), getmsgtype(msg),
                                         getmsgid(msg), payload);
                    }
                }
            }
            void
            timercb(idref id, unsigned& ms)
            {
#if ENABLE_MULTICAST
                if (id == mwait_.id()) {

                    aug_ctxinfo(aug_tlx, "process cluster timer");
                    mflush();
                    ms = cluster_.expiry();

                } else
#endif // ENABLE_MULTICAST
                {
                    AUG_CTXDEBUG2(aug_tlx, "custom timer expiry");

                    sessiontimers::iterator it(sessiontimers_.find(id.get()));
                    mod_handle timer = { id.get(), it->second.ob_.get() };
                    it->second.session_->expire(timer, ms);

                    if (0 == ms) {
                        AUG_CTXDEBUG2(aug_tlx,
                                      "removing timer:"
                                      " ms has been set to zero");
                        sessiontimers_.erase(it);
                    }
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

#if ENABLE_MULTICAST
            void
            mflush()
            {
                aug_packet pkt;
                while (cluster_.next(pkt)) {

                    aug_ctxinfo(aug_tlx, "mflush message [%u]",
                                static_cast<unsigned>(pkt.seqno_));

                    string type;
                    if (!types_.getbyid(pkt.type_, type))
                        continue;
                    blobptr blob(createblob(getmpool(aug_tlx), pkt.data_,
                                            pkt.size_));

                    // Add uri scheme to session name.

                    string from("node:");
                    from += pkt.node_;

                    vector<sessionptr> sessions;
                    sessions_.getsessions(sessions);

                    vector<sessionptr>::const_iterator it(sessions.begin()),
                        end(sessions.end());
                    for (; it != end; ++it)
                        (*it)->event(from.c_str(), type.c_str(), 0, blob);
                }
# if LOGCLUSTER_
                stringstream ss;
                cluster_.print(ss);
                aug_ctxinfo(aug_tlx, "%s", ss.str().c_str());
# endif // LOGCLUSTER_
            }
            void
            mrecv()
            {
                char buf[AUG_PACKETSIZE];
                endpoint from(null);
                if (AUG_PACKETSIZE
                    != recvfrom(mcastsd_, buf, sizeof(buf), 0, from))
                    throw aug_error(__FILE__, __LINE__, AUG_EIO,
                                    AUG_MSG("bad packet size"));
                aug_packet pkt;
                verify(aug_decodepacket(buf, &pkt));
                cluster_.insert(pkt);
            }
            void
            mprocess()
            {
                try {
                    for (;;)
                        mrecv();
                } catch (const block_exception&) {
                }
                mflush();
                mwait_.set(cluster_.expiry(), *this);
            }
#endif // ENABLE_MULTICAST
        };
    }
}

AUGASPP_API
engine::~engine() AUG_NOTHROW
{
    delete impl_;
}

AUGASPP_API
engine::engine(const char* node, aug_muxer_t muxer, aug_events_t events,
               aug_timers_t timers, enginecb_base& cb, unsigned timeout)
    : impl_(new (tlx) detail::engineimpl(node, muxer, events, timers, cb,
                                         timeout))
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
engine::join(const char* addr, unsigned short port, const char* ifname)
{
#if ENABLE_MULTICAST
    inetaddr in(addr);

    autosd sd(aug::socket(family(in), SOCK_DGRAM));
    setnonblock(sd, true);
    setreuseaddr(sd, true);

    // Set outgoing multicast interface.

    if (ifname)
        setmcastif(sd, ifname);

    const endpoint any(inetany(family(in)), htons(port));
    aug::bind(sd, any);

    joinmcast(sd, in, ifname);

    setfamily(impl_->mcastep_, family(in));
    setport(impl_->mcastep_, htons(port));
    setinetaddr(impl_->mcastep_, in);

    // TODO: is this needed?

    impl_->mwait_.set(impl_->cluster_.expiry(), *impl_);

	setmdeventmask(impl_->muxer_, sd, AUG_MDEVENTRDEX);
    impl_->mcastsd_ = sd;
#endif // ENABLE_MULTICAST
}

AUGASPP_API void
engine::insert(const string& name, const sessionptr& session,
               const char* topics)
{
    impl_->sessions_.insert(name, session, topics);
}

AUGASPP_API void
engine::insert(unsigned id, const std::string& name)
{
#if ENABLE_MULTICAST
    impl_->types_.insert(id, name);
#endif // ENABLE_MULTICAST
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

            aug_timeval tv;
            processexpired(impl_->timers_, 0 == ready, tv);

            try {

                ready = getreadychans(impl_->chans_);
                if (ready) {

                    // Some are ready so don't block.

                    pollmdevents(impl_->muxer_);

                } else if (aug_timersempty(impl_->timers_)) {

                    // No timers so wait indefinitely.

                    scoped_sigunblock unblock;
                    ready = waitmdevents(impl_->muxer_);

                } else {

                    // Wait upto next timer expiry.

                    scoped_sigunblock unblock;
                    ready = waitmdevents(impl_->muxer_, tv);
                }

            } catch (const intr_exception&) {
                ready = !0; // Not timeout.
                continue;
            }

            // Update timestamp after waiting.

            gettimeofday(getclock(aug_tlx), impl_->now_);

#if ENABLE_MULTICAST
            AUG_CTXDEBUG2(aug_tlx, "processing multicast");

            if (getmdevents(impl_->muxer_, impl_->mcastsd_))
                impl_->mprocess();
#endif // ENABLE_MULTICAST

            AUG_CTXDEBUG2(aug_tlx, "processing events");

            if (getmdevents(impl_->muxer_, eventsmd(impl_->events_)))
                try {

                    // Read events until operation would block.

                    for (;;)
                        impl_->readevent();

                } catch (const block_exception&) {
                }

            AUG_CTXDEBUG2(aug_tlx, "processing files");

            processchans(impl_->chans_);
            continue;

        } AUG_PERRINFOCATCH;

        // Stop on error only when running in foreground.

        if (stoponerr)
            impl_->socks_.teardown(impl_->chans_, impl_->now_);
    }
}

AUGASPP_API void
engine::reconfall()
{
    // Thread-safe.

    aug_event e = { AUG_EVENTRECONF, 0 };
    writeevent(impl_->events_, e);
}

AUGASPP_API void
engine::stopall()
{
    // Thread-safe.

    aug_event e = { AUG_EVENTSTOP, 0 };
    writeevent(impl_->events_, e);
}

AUGASPP_API void
engine::post(const char* sname, const char* to, const char* type, mod_id id,
             objectref ob)
{
    // Thread-safe.

    // The 'to' descriptor must either be a topic name or a topic uri.  Remove
    // uri scheme if present.

    msgptr msg(msgevent::create(id, sname, gettopic(to), type));
    setmsgpayload(msg, ob);
    aug_event e;
    e.type_ = POSTEVENT_;
    e.ob_ = msg.base();

    writeevent(impl_->events_, e);
}

AUGASPP_API void
engine::dispatch(const char* sname, const char* to, const char* type,
                 mod_id id, objectref ob)
{
    // Add uri scheme to session name.

    string from("topic:");
    from += sname;

    vector<sessionptr> sessions;

    // The 'to' descriptor must either be a topic name or a topic uri.  Remove
    // uri scheme if present.

    impl_->sessions_.getbytopic(sessions, gettopic(to));

    vector<sessionptr>::const_iterator it(sessions.begin()),
        end(sessions.end());
    for (; it != end; ++it)
        (*it)->event(from.c_str(), type, id, ob);
}

AUGASPP_API void
engine::shutdown(mod_id cid, unsigned flags)
{
    chanptr chan(impl_->findchan(cid));
    sockptr sock(impl_->socks_.get(cid));

    sock->shutdown(chan, flags, impl_->now_);
}

AUGASPP_API mod_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   sslctx* ctx, objectref ob)
{
#if !WITH_SSL
    if (ctx)
        throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                        AUG_MSG("ssl not supported"));
#endif // !WITH_SSL

    sessionptr session(impl_->sessions_.getbyname(sname));

    mpoolptr mpool(getmpool(aug_tlx));
    chanptr chan(createclient(mpool, host, port, impl_->muxer_,
                              ctx ? ctx->get() : 0));
    connptr conn(new (tlx) clntconn(mpool, session, impl_->timers_,
                                    getchanid(chan), ob));

    impl_->socks_.insert(conn);
    insertchan(impl_->chans_, chan);
    return conn->id();
}

AUGASPP_API mod_id
engine::tcplisten(const char* sname, const char* host, const char* port,
                  sslctx* ctx, objectref ob)
{
#if !WITH_SSL
    if (ctx)
        throw aug_error(__FILE__, __LINE__, AUG_ESUPPORT,
                        AUG_MSG("ssl not supported"));
#endif // !WITH_SSL

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
    listenerptr conn(new (tlx) listener(session, getchanid(chan), ob));

    impl_->socks_.insert(conn);
    insertchan(impl_->chans_, chan);
    return conn->id();
}

AUGASPP_API void
engine::send(mod_id cid, const void* buf, size_t len)
{
    chanptr chan(impl_->findchan(cid));
    sockptr sock(impl_->socks_.get(cid));
    connptr conn(smartptr_cast<conn_base>(sock));

    if (null == conn || !sendable(*conn))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");

    conn->send(chan, buf, len, impl_->now_);
}

AUGASPP_API void
engine::sendv(mod_id cid, blobref blob)
{
    chanptr chan(impl_->findchan(cid));
    sockptr sock(impl_->socks_.get(cid));
    connptr conn(smartptr_cast<conn_base>(sock));

    if (null == conn || !sendable(*conn))
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection has been shutdown");

    conn->sendv(chan, blob, impl_->now_);
}

AUGASPP_API void
engine::setrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_EEXIST,
                        "connection not found: id=[%d]",
                        static_cast<int>(cid));
    rwtimer->setrwtimer(ms, flags);
}

AUGASPP_API bool
engine::resetrwtimer(mod_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]",
                        static_cast<int>(cid));

    return rwtimer->resetrwtimer(ms, flags);
}

AUGASPP_API bool
engine::cancelrwtimer(mod_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.get(cid)));
    if (null == rwtimer)
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        "connection not found: id=[%d]",
                        static_cast<int>(cid));
    return rwtimer->cancelrwtimer(flags);
}

AUGASPP_API mod_id
engine::settimer(const char* sname, unsigned ms, objectref ob)
{
    mod_id id(aug_nextid());
    boxptrptr local(createboxptr(getmpool(aug_tlx), impl_, 0));

    aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                  &detail::engineimpl::timercb>, local);

    // Insert after settimer() has succeeded.

    detail::sessiontimer timer(impl_->sessions_.getbyname(sname), ob);
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

AUGASPP_API void
engine::emit(const char* type, const void* buf, size_t len)
{
#if ENABLE_MULTICAST
    unsigned id;
    if (impl_->types_.getbyname(type, id))
        impl_->mpacket_.emit(impl_->mcastsd_, id, buf, len, impl_->mcastep_);
#else // !ENABLE_MULTICAST
    blobptr blob(createblob(getmpool(aug_tlx), buf, len));

    // Add uri scheme to session name.

    string from("node:");
    from += impl_->node_;

    vector<sessionptr> sessions;
    impl_->sessions_.getsessions(sessions);

    vector<sessionptr>::const_iterator it(sessions.begin()),
        end(sessions.end());
    for (; it != end; ++it)
        (*it)->event(from.c_str(), type, 0, blob);
#endif // !ENABLE_MULTICAST
}

AUGASPP_API bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
