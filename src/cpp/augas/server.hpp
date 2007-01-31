/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SERVER_HPP
#define AUGAS_SERVER_HPP

#include "augas/buffer.hpp"
#include "augas/conn.hpp"

namespace augas {

    class server : public rwtimer_base, public conn_base {

        augas_object sock_;
        buffer buffer_;
        rwtimer rwtimer_;
        established conn_;

        // rwtimer_base.

        void
        do_callback(aug::idref ref, unsigned& ms, aug_timers& timers);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned ms, unsigned flags);

        void
        do_resetrwtimer(unsigned flags);

        void
        do_cancelrwtimer(unsigned flags);

        // conn_base.

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const sessptr&
        do_sess() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(aug::mplexer& mplexer);

        void
        do_putsome(aug::mplexer& mplexer, const void* buf, size_t size);

        void
        do_shutdown();

        void
        do_teardown();

        const aug::endpoint&
        do_endpoint() const;

        connphase
        do_phase() const;

    public:
        ~server() AUG_NOTHROW;

        server(const sessptr& sess, void* user, aug::timers& timers,
               const aug::smartfd& sfd, const aug::endpoint& ep);
    };
}

#endif // AUGAS_SERVER_HPP
