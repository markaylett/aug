/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_CLNTCONN_HPP
#define DAUG_CLNTCONN_HPP

#include "daug/buffer.hpp"
#include "daug/conn.hpp"

namespace augas {

    class clntconn : public rwtimer_base, public conn_base {

        augas_object sock_;
        buffer buffer_;
        rwtimer rwtimer_;
        connptr conn_;

        // rwtimer_base.

        void
        do_timercb(int id, unsigned& ms);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned flags);

        bool
        do_cancelrwtimer(unsigned flags);

        // conn_base.

        augas_object&
        do_object();

        const augas_object&
        do_object() const;

        const servptr&
        do_serv() const;

        aug::smartfd
        do_sfd() const;

        bool
        do_accept(const aug_endpoint& ep);

        void
        do_append(const aug_var& var);

        void
        do_append(const void* buf, size_t size);

        void
        do_connected(const aug_endpoint& ep);

        bool
        do_process(unsigned short events);

        void
        do_shutdown();

        void
        do_teardown();

        bool
        do_authcert(const char* subject, const char* issuer);

        const aug::endpoint&
        do_endpoint() const;

        connphase
        do_phase() const;

    public:
        ~clntconn() AUG_NOTHROW;

        clntconn(const servptr& serv, void* user, aug::timers& timers,
                 const char* host, const char* port);
    };
}

#endif // DAUG_CLNTCONN_HPP
