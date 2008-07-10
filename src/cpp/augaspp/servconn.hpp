/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_SERVCONN_HPP
#define AUGASPP_SERVCONN_HPP

#include "augaspp/buffer.hpp"
#include "augaspp/conn.hpp"

namespace aug {

    class servconn : public rwtimer_base, public conn_base {

        mod_handle sock_;
        buffer buffer_;
        rwtimer rwtimer_;
        aug::connected conn_;

        // rwtimer_base.

        void
        do_timercb(idref id, unsigned& ms);

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

        chanptr
        do_chan() const;

        void
        do_send(const void* buf, size_t size, const timeval& now);

        void
        do_sendv(blobref ref, const timeval& now);

        bool
        do_accepted(const std::string& name, const timeval& now);

        void
        do_connected(const std::string& name, const timeval& now);

        bool
        do_process(obref<aug_stream> stream, unsigned short events,
                   const timeval& now);

        void
        do_shutdown(unsigned flags, const timeval& now);

        void
        do_teardown(const timeval& now);

        bool
        do_authcert(const char* subject, const char* issuer);

        std::string
        do_peername() const;

        sockstate
        do_state() const;

    public:
        ~servconn() AUG_NOTHROW;

        servconn(const sessionptr& session, void* user, timers& timers,
                 const chanptr& chan);
    };
}

#endif // AUGASPP_SERVCONN_HPP
