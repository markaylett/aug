/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONN_HPP
#define AUGAS_CONN_HPP

#include "augas/buffer.hpp"
#include "augas/sess.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"

namespace augas {

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
        putsome(aug::mplexer& mplexer, const void* buf, size_t size);

        void
        shutdown();

        void
        setrwtimer(unsigned ms, unsigned flags);

        void
        resetrwtimer(unsigned ms, unsigned flags);

        void
        cancelrwtimer(unsigned flags);

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
        const char*
        sname() const
        {
            return sess_->name();
        }
    };

    typedef aug::smartptr<conn> connptr;
}

#endif // AUGAS_CONN_HPP
