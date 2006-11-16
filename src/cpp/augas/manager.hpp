/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MANAGER_HPP
#define AUGAS_MANAGER_HPP

#include "augas/buffer.hpp"
#include "augas/module.hpp"

#include <augutilpp.hpp>
#include <augsyspp.hpp>

#include <algorithm>
#include <map>
#include <string>

namespace augas {

    class options;

    class sess {

        moduleptr module_;
        augas_sess sess_;
        bool open_;

    public:
        ~sess() AUG_NOTHROW;

        sess(const moduleptr& module, const char* name);

        void
        open();

        bool
        isopen() const
        {
            return open_;
        }
        void
        event(int type, void* user) const
        {
            module_->event(sess_, type, user);
        }
        void
        expire(unsigned tid, void* user, unsigned& ms) const
        {
            module_->expire(sess_, tid, user, ms);
        }
        void
        reconf() const
        {
            module_->reconf(sess_);
        }
        void
        closeconn(const augas_conn& conn) const
        {
            module_->closeconn(conn);
        }
        void
        openconn(augas_conn& conn, const char* addr,
                 unsigned short port) const
        {
            module_->openconn(conn, addr, port);
        }
        void
        notconn(const augas_conn& conn) const
        {
            module_->notconn(conn);
        }
        void
        data(const augas_conn& conn, const char* buf, size_t size) const
        {
            module_->data(conn, buf, size);
        }
        void
        rdexpire(const augas_conn& conn, unsigned& ms) const
        {
            module_->rdexpire(conn, ms);
        }
        void
        wrexpire(const augas_conn& conn, unsigned& ms) const
        {
            module_->wrexpire(conn, ms);
        }
        void
        teardown(const augas_conn& conn) const
        {
            module_->teardown(conn);
        }
    };

    typedef aug::smartptr<sess> sessptr;

    class conn : public aug::timercb_base {

        sessptr sess_;
        aug::smartfd sfd_;
        augas_conn conn_;
        aug::timer rdtimer_;
        aug::timer wrtimer_;
        buffer buffer_;
        bool shutdown_;

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

    public:
        ~conn() AUG_NOTHROW;

        conn(const sessptr& sess, const aug::smartfd& sfd, augas_id cid,
             const aug_endpoint& ep, aug::timers& timers);

        bool
        process(aug::mplexer& mplexer);

        void
        putsome(aug::mplexer& mplexer, const void* buf, size_t size)
        {
            buffer_.putsome(buf, size);
            setioeventmask(mplexer, sfd_, AUG_IOEVENTRDWR);
        }

        void
        shutdown()
        {
            shutdown_ = true;
            if (buffer_.empty())
                aug::shutdown(sfd_, SHUT_WR);
        }
        void
        setrwtimer(unsigned ms, unsigned flags)
        {
            if (flags & AUGAS_TIMRD)
                rdtimer_.set(ms, *this);

            if (flags & AUGAS_TIMWR)
                wrtimer_.set(ms, *this);
        }
        void
        resetrwtimer(unsigned ms, unsigned flags)
        {
            if (flags & AUGAS_TIMRD)
                rdtimer_.reset(ms);

            if (flags & AUGAS_TIMWR)
                wrtimer_.reset(ms);
        }
        void
        cancelrwtimer(unsigned flags)
        {
            if (flags & AUGAS_TIMRD)
                rdtimer_.cancel();

            if (flags & AUGAS_TIMWR)
                wrtimer_.cancel();
        }
        void
        notconn() const
        {
            sess_->notconn(conn_);
        }
        void
        data(const char* buf, size_t size) const
        {
            sess_->data(conn_, buf, size);
        }
        void
        rdexpire(unsigned& ms) const
        {
            sess_->rdexpire(conn_, ms);
        }
        void
        wrexpire(unsigned& ms) const
        {
            sess_->wrexpire(conn_, ms);
        }
        void
        teardown() const
        {
            sess_->teardown(conn_);
        }
        bool
        isshutdown() const
        {
            return shutdown_;
        }
        augas_id
        id() const
        {
            return conn_.id_;
        }
        int
        fd() const
        {
            return sfd_.get();
        }
    };

    typedef aug::smartptr<conn> connptr;

    typedef std::map<std::string, moduleptr> modules;
    typedef std::map<std::string, sessptr> sesss;
    typedef std::map<int, std::pair<sessptr, aug::smartfd> > listeners;
    typedef std::map<augas_id, int> idtofd;
    typedef std::map<int, connptr> conns;

    struct manager {

        modules modules_;
        sesss sesss_;
        listeners listeners_;
        idtofd idtofd_;
        conns conns_;

        void
        erase(const connptr& conn);

        void
        insert(const connptr& conn);

        void
        insert(const sessptr& sess, const aug::smartfd& sfd);

        void
        teardown();

        void
        load(const options& options, const augas_host& host);

        bool
        sendall(aug::mplexer& mplexer, augas_id cid, const char* sname,
                const char* buf, size_t size);

        bool
        sendself(aug::mplexer& mplexer, augas_id cid, const char* buf,
                 size_t size);

        void
        sendother(aug::mplexer& mplexer, augas_id cid, const char* sname,
                  const char* buf, size_t size);

        connptr
        getbyfd(aug::fdref fd) const;

        connptr
        getbyid(augas_id id) const;

        sessptr
        getsess(const std::string& name) const;

        bool
        isconnected() const;

        sessptr
        islistener(aug::fdref fd) const;

        void
        reconf() const;
    };
}

#endif // AUGAS_MANAGER_HPP
