/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_CLNTCONN_HPP
#define AUGRTPP_CLNTCONN_HPP

#include "augaspp/buffer.hpp"
#include "augaspp/conn.hpp"

namespace aug {

    class clntconn : public rwtimer_base, public conn_base {

        mod_handle sock_;
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

        mod_handle&
        do_get();

        const mod_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        void
        do_send(const void* buf, size_t size, const timeval& now);

        void
        do_sendv(blobref ref, const timeval& now);

        bool
        do_accepted(const aug_endpoint& ep, const timeval& now);

        void
        do_connected(const aug_endpoint& ep, const timeval& now);

        bool
        do_process(unsigned short events, const timeval& now);

        void
        do_shutdown(unsigned flags, const timeval& now);

        void
        do_teardown(const timeval& now);

        bool
        do_authcert(const char* subject, const char* issuer);

        const endpoint&
        do_peername() const;

        sockstate
        do_state() const;

    public:
        ~clntconn() AUG_NOTHROW;

        clntconn(const sessionptr& session, void* user, timers& timers,
                 const char* host, const char* port);
    };
}

#endif // AUGRTPP_CLNTCONN_HPP
