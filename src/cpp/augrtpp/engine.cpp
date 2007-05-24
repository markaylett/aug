/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/engine.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrtpp/clntconn.hpp"
#include "augrtpp/listener.hpp"
#include "augrtpp/servs.hpp"
#include "augrtpp/socks.hpp"
#include "augrtpp/ssl.hpp"

#include "augnetpp/nbfile.hpp"

#include "augutilpp/timer.hpp"

#include <map>
#include <queue>

using namespace aug;
using namespace std;

namespace {
    typedef map<int, pair<servptr, aug_var> > timerpairs;
    typedef queue<connptr> pending;
}

namespace aug {
    namespace detail {
        struct engineimpl {

            fdref eventfd_;
            aug_nbfilecb_t cb_;
            aug_var var_;
            nbfiles nbfiles_;
            servs servs_;
            socks socks_;
            timers timers_;
            timer grace_;

            // Mapping of timer-ids to services.

            timerpairs timerpairs_;

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
                    removenbfile(eventfd_);
                } AUG_PERRINFOCATCH;
            }
            engineimpl(fdref eventfd, aug_nbfilecb_t cb, const aug_var& var)
                : eventfd_(eventfd),
                  cb_(cb),
                  var_(var),
                  grace_(timers_),
                  state_(STARTED)
            {
                AUG_DEBUG2("inserting event pipe to list");
                insertnbfile(nbfiles_, eventfd, cb, var);
                setnbeventmask(eventfd, AUG_FDEVENTRD);
            }
            void
            customcb(int id, unsigned& ms)
            {
                AUG_DEBUG2("custom timer expiry");

                timerpairs::iterator it(timerpairs_.find(id));
                pair<servptr, aug_var> xy(it->second);
                augas_object timer = { id, xy.second.arg_ };
                xy.first->expire(timer, ms);

                if (0 == ms) {
                    AUG_DEBUG2("removing timer: ms has been set to zero");
                    timerpairs_.erase(it);
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

engine::~engine() AUG_NOTHROW
{
    delete impl_;
}

engine::engine(fdref eventfd, aug_nbfilecb_t cb, const aug_var& var)
    : impl_(new detail::engineimpl(eventfd, cb, var))
{
}

void
engine::dispatch(const char* sname, const char* to, const char* type,
                 const void* user, size_t size)
{
    vector<servptr> servs;
    impl_->servs_.getbygroup(servs, to);

    vector<servptr>::const_iterator it(servs.begin()), end(servs.end());
    for (; it != end; ++it)
        (*it)->event(sname, type, user, size);
}

void
engine::shutdown_(augas_id cid)
{
    sockptr sock(impl_->socks_.getbyid(cid));
    connptr cptr(smartptr_cast<conn_base>(sock));
    if (null != cptr)
        cptr->shutdown();
    else
        impl_->socks_.erase(*sock);
}

void
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

augas_id
engine::tcpconnect(const char* sname, const char* host, const char* port,
                   void* user)
{
    servptr serv(impl_->servs_.getbyname(sname));
    connptr cptr(new clntconn(serv, user, impl_->timers_, host, port));

    // Remove on exception.

    scoped_insert si(impl_->socks_, cptr);

    if (CONNECTED == cptr->state()) {

        // connected() must only be called after this function has returned.

        insertnbfile(impl_->nbfiles_, cptr->sfd(), impl_->cb_, impl_->var_);
        setnbeventmask(cptr->sfd(), AUG_FDEVENTRD);

        if (impl_->pending_.empty()) {

            // Schedule an event to ensure that connected() is called after
            // this function has returned.

            aug_event e = { AUG_EVENTWAKEUP, AUG_VARNULL };
            writeevent(impl_->eventfd_, e);
        }

        // Add to pending queue.

        impl_->pending_.push(cptr);

    } else {

        insertnbfile(impl_->nbfiles_, cptr->sfd(), impl_->cb_, impl_->var_);
        setnbeventmask(cptr->sfd(), AUG_FDEVENTALL);
    }

    si.commit();
    return id(*cptr);
}

augas_id
engine::tcplisten(const char* sname, const char* host, const char* port,
                  void* user)
{
    // Bind listener socket.

    endpoint ep(null);
    smartfd sfd(aug::tcplisten(host, port, ep));

    insertnbfile(impl_->nbfiles_, sfd, impl_->cb_, impl_->var_);
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

void
engine::send(augas_id cid, const void* buf, size_t len)
{
    if (!impl_->socks_.append(cid, buf, len))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

void
engine::sendv(augas_id cid, const augas_var& var)
{
    if (!impl_->socks_.append(cid, var))
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection has been shutdown");
}

void
engine::setrwtimer(augas_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_EEXIST,
                          "connection not found: id=[%d]", cid);
    rwtimer->setrwtimer(ms, flags);
}

bool
engine::resetrwtimer(augas_id cid, unsigned ms, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);

    return rwtimer->resetrwtimer(ms, flags);
}

bool
engine::cancelrwtimer(augas_id cid, unsigned flags)
{
    rwtimerptr rwtimer(smartptr_cast<
                       rwtimer_base>(impl_->socks_.getbyid(cid)));
    if (null == rwtimer)
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          "connection not found: id=[%d]", cid);
    return rwtimer->cancelrwtimer(flags);
}

augas_id
engine::settimer(const char* sname, unsigned ms, const augas_var& var)
{
    augas_id id(aug_nextid());

    // If aug_settimer() succeeds, it will call aug_destroyvar() on var when
    // the timer is destroyed.  The service is added to the container first to
    // minimise any chance of failure after aug_settimer() has been called.

    impl_->timerpairs_[id] = make_pair(impl_->servs_.getbyname(sname), var);

    try {
        aug_var local = { 0, impl_ };
        aug::settimer(impl_->timers_, id, ms, timermemcb<detail::engineimpl,
                      &detail::engineimpl::customcb>, local);
    } catch (...) {
        impl_->timerpairs_.erase(id);
        throw;
    }
    return id;
}

bool
engine::resettimer(augas_id tid, unsigned ms)
{
    return aug::resettimer(impl_->timers_, tid, ms);
}

bool
engine::canceltimer(augas_id tid)
{
    bool ret(aug::canceltimer(impl_->timers_, tid));

    // Only erase if aug_canceltimer() returns true: it may be in the midst of
    // a aug_foreachexpired() call, in which case, aug_canceltimer() will
    // return false for the timer being expired.

    if (ret)
        impl_->timerpairs_.erase(tid);
    return ret;
}

bool
engine::stopping() const
{
    return detail::engineimpl::STARTED != impl_->state_;
}
