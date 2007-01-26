/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CLIENT_HPP
#define AUGAS_CLIENT_HPP

#include "augas/conn.hpp"

namespace augas {

    class client : public rwtimer_base, public conn_base {

        augas_sock sock_;
        rwtimer rwtimer_;
        connptr conn_;

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

        augas_sock&
        do_sock();

        const augas_sock&
        do_sock() const;

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
        ~client() AUG_NOTHROW;

        client(const sessptr& sess, void* user, aug::timers& timers,
               const char* host, const char* serv);
    };
}

#endif // AUGAS_CLIENT_HPP
