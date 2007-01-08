/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONN_HPP
#define AUGAS_CONN_HPP

#include "augas/buffer.hpp"
#include "augas/file.hpp"

#include "augsyspp.hpp"
#include "augutilpp.hpp"

namespace augas {

    class conn : public file_base, public aug::timercb_base {
    public:
        typedef augas_conn ctype;
    private:

        sessptr sess_;
        aug::smartfd sfd_;
        augas_conn conn_;
        aug::timer rdtimer_;
        aug::timer wrtimer_;
        buffer buffer_;
        bool open_, teardown_, shutdown_;

        augas_id
        do_id() const;

        int
        do_fd() const;

        const sessptr&
        do_sess() const;

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

    public:
        ~conn() AUG_NOTHROW;

        conn(const sessptr& sess, const aug::smartfd& sfd, augas_id cid,
             aug::timers& timers);

        void
        open(const aug_endpoint& ep);

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
        teardown();

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
        operator augas_conn&()
        {
            return conn_;
        }
        operator const augas_conn&() const
        {
            return conn_;
        }
        bool
        isopen() const
        {
            return open_;
        }
        bool
        isshutdown() const
        {
            return shutdown_;
        }
    };

    typedef aug::smartptr<conn> connptr;
}

#endif // AUGAS_CONN_HPP
