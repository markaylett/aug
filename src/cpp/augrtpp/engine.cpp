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
#include "augrtpp/servs.hpp"
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

namespace {

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

    struct servtimer {
        servptr serv_;
        aug_var var_;
        ~servtimer() AUG_NOTHROW
        {
            try {
                destroyvar(var_);
            } AUG_PERRINFOCATCH;
        }
        explicit
        servtimer(const servptr& serv)
            : serv_(serv)
        {
            var_.type_ = 0;
            var_.arg_ = 0;
        }
    };

    typedef smartptr<servtimer> servtimerptr;
    typedef map<augas_id, servtimerptr> servtimers;
    typedef queue<connptr> pending;

    void
    setsockopts(const smartfd& sfd)
    {
        setnodelay(sfd, true);
        setnonblock(sfd, true);
    }

    void
    setconnected(conn_base& conn)
    {
        setsockopts(conn.sfd());

        const endpoint& ep(conn.peername());
        inetaddr addr(null);
        AUG_DEBUG2("connected: host=[%s], port=[%d]",
                   inetntop(getinetaddr(ep, addr)).c_str(),
                   static_cast<int>(ntohs(port(ep))));

        setnbeventmask(conn.sfd(), AUG_FDEVENTRD);
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

            fdref rdfd_, wrfd_;
            enginecb_base& cb_;
            nbfiles nbfiles_;
            servs servs_;
            socks socks_;
            timers timers_;
            timer grace_;

            // Mapping of timer-ids to services.

            servtimers servtimers_;

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
                    removenbfile(rdfd_);
                } AUG_PERRINFOCATCH;
            }
            engineimpl(fdref rdfd, fdref wrfd, enginecb_base& cb)
                : rdfd_(rdfd),
                  wrfd_(wrfd),
                  cb_(cb),
                  grace_(timers_),
                  state_(STARTED)
            {
                AUG_DEBUG2("inserting event pipe to list");
                insertnbfile(nbfiles_, rdfd, *this);
                setnbeventmask(rdfd, AUG_FDEVENTRD);
            }
            void
            teardown()
            {
                if (STARTED == state_) {

                    state_ = TEARDOWN;
                    socks_.teardown();

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
                connptr cptr(new servconn(sock.serv(), user(sock), timers_,
                                          sfd, ep));

                scoped_insert si(socks_, cptr);
                AUG_DEBUG2("initialising connection: id=[%d], fd=[%d]",
                           id(*cptr), sfd.get());

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

                switch (aug::readevent(rdfd_, event).type_) {
                case AUG_EVENTRECONF:
                    AUG_DEBUG2("received AUG_EVENTRECONF");
                    cb_.reconf();
                    servs_.reconf();
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

                        vector<servptr> servs;
                        servs_.getbygroup(servs, ev->to_);

                        size_t size;
                        const void* user(varbuf(ev->var_, size));

                        vector<servptr>::const_iterator it(servs.begin()),
                            end(servs.end());
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

                if (fd == rdfd_)
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

                servtimers::iterator it(servtimers_.find(id));
                servtimerptr tptr(it->second);
                augas_object timer = { id, tptr->var_.arg_ };
                tptr->serv_->expire(timer, ms);

                if (0 == ms) {
                    AUG_DEBUG2("removing timer: ms has been set to zero");
                    servtimers_.erase(it);
                }
            }
            void
            reopencb(int id, unsigned& ms)
            {
                cb_.reopen();
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
engine::engine(fdref rdfd, fdref wrfd, enginecb_base& cb)
    : impl_(new detail::engineimpl(rdfd, wrfd, cb))
{
}

AUGRTPP_API void
engine::clear()
{
    impl_->socks_.clear();

    // TODO: erase the services in reverse order to which they were added.

    impl_->servs_.clear();
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
    writeevent(impl_->wrfd_, e);

    // Event takes ownership of var when post cannot fail.

    aug_setvar(&arg->var_, var);
    arg.release();
}

AUGRTPP_API void
engine::dispatch(const char* sname, const char* to, const char* type,
                 const void* user, size_t size)
{
    vector<servptr> servs;
    impl_->servs_.getbygroup(servs, to);

    vector<servptr>::const_iterator it(servs.begin()), end(servs.end());
    for (; it != end; ++it)
        (*it)->event(sname, type, user, size);
}

AUGRTPP_API void
engine::shutdown(augas_id cid)
{
    sockptr sock(impl_->socks_.getbyid(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    if (null != cptr)
        cptr->shutdown();
    else
        impl_->socks_.erase(*sock);
}

AUGRTPP_API void
engine::teardown()
{
    if (detail::engineimpl::STARTED == impl_->state_) {

        impl_->state_ = detail::engineimpl::TEARDOWN;
        impl_->socks_.teardown();

        aug_var var = { 0, impl_ };
        impl_->grace_.set(15000, timermemcb<detail::engineimpl,
                          &detail::engineimpl::stopcb>, var);
    }
}

AUGRTPP_API augas_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   void* user)
{
    servptr serv(impl_->servs_.getbyname(sname));
    connptr cptr(new clntconn(serv, user, impl_->timers_, host, port));

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
            writeevent(impl_->wrfd_, e);
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

AUGRTPP_API augas_id
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

    servptr serv(impl_->servs_.getbyname(sname));
    listenerptr lptr(new listener(serv, user, sfd));
    scoped_insert si(impl_->socks_, lptr);

    si.commit();
    return id(*lptr);
}

AUGRTPP_API void
engine::send(augas_id cid, const void* buf, size_t len)
{
    if (!impl_->socks_.send(cid, buf, len))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

AUGRTPP_API void
engine::sendv(augas_id cid, const aug_var& var)
{
    if (!impl_->socks_.sendv(cid, var))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

AUGRTPP_API void
engine::setrwtimer(augas_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_EEXIST,
                          "connection not found: id=[%d]", cid);
    rwtimer->setrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::resetrwtimer(augas_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    return rwtimer->resetrwtimer(ms, flags);
}

AUGRTPP_API bool
engine::cancelrwtimer(augas_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);
    return rwtimer->cancelrwtimer(flags);
}

AUGRTPP_API augas_id
engine::settimer(const char* sname, unsigned ms, const aug_var* var)
{
    augas_id id(aug_nextid());
    aug_var local = { 0, impl_ };

    // If aug_settimer() succeeds, aug_destroyvar() will be called on var when
    // the timer is destroyed.

    aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                  &detail::engineimpl::timercb>, local);

    servtimerptr tptr(new servtimer(impl_->servs_.getbyname(sname)));
    impl_->servtimers_[id] = tptr;

    // Timer takes ownership of var when settimer cannot fail.

    aug_setvar(&tptr->var_, var);
    return id;
}

AUGRTPP_API bool
engine::resettimer(augas_id tid, unsigned ms)
{
    return aug::resettimer(impl_->timers_, tid, ms);
}

AUGRTPP_API bool
engine::canceltimer(augas_id tid)
{
    bool ret(aug::canceltimer(impl_->timers_, tid));

    // Only erase if aug_canceltimer() returns true: it may be in the midst of
    // a aug_foreachexpired() call, in which case, aug_canceltimer() will
    // return false for the timer being expired.

    if (ret)
        impl_->servtimers_.erase(tid);
    return ret;
}

AUGRTPP_API void
engine::setsslclient(augas_id cid, sslctx& ctx)
{
#if HAVE_OPENSSL_SSL_H
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    aug::setsslclient(*cptr, ctx);
#else // !HAVE_OPENSSL_SSL_H
    throw local_error(__FILE__, __LINE__, AUG_ESUPPORT,
                      AUG_MSG("aug_setsslclient() not supported"));
#endif // !HAVE_OPENSSL_SSL_H
}

AUGRTPP_API void
engine::setsslserver(augas_id cid, sslctx& ctx)
{
#if HAVE_OPENSSL_SSL_H
    connptr cptr(smartptr_cast<
                 conn_base>(impl_->socks_.getbyid(cid)));
    if (null == cptr)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    aug::setsslserver(*cptr, ctx);
#else // !HAVE_OPENSSL_SSL_H
    throw local_error(__FILE__, __LINE__, AUG_ESUPPORT,
                      AUG_MSG("aug_setsslserver() not supported"));
#endif // !HAVE_OPENSSL_SSL_H
}

AUGRTPP_API void
engine::insert(const string& name, const servptr& serv, const char* groups)
{
    impl_->servs_.insert(name, serv, groups);
}

AUGRTPP_API void
engine::cancelinactive()
{
    // Remove any timers allocated to services that could not be opened.

    servtimers::iterator it(impl_->servtimers_.begin()),
        end(impl_->servtimers_.end());
    while (it != end) {
        if (!it->second->serv_->active()) {
            aug_warn("cancelling timer associated with inactive service");
            aug_canceltimer(cptr(impl_->timers_), it->first);
            impl_->servtimers_.erase(it++);
        } else
            ++it;
    }
}

AUGRTPP_API void
engine::run(bool daemon)
{
    timer reopen(impl_->timers_);

    // Re-open log file every minute.

    if (daemon) {

        aug_var var = { 0, impl_ };
        reopen.set(15000, timermemcb<detail::engineimpl,
                   &detail::engineimpl::reopencb>, var);
    }

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

        if (!daemon)
            teardown();
    }
}

AUGRTPP_API bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
