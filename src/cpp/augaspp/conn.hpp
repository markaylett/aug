/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_CONN_HPP
#define AUGRTPP_CONN_HPP

#include "augaspp/rwtimer.hpp"
#include "augaspp/sock.hpp"

#include "augnetpp.hpp"

namespace aug {

    class buffer;

    class conn_base : public sock_base {

        virtual void
        do_send(const void* buf, size_t size, const timeval& now) = 0;

        virtual void
        do_sendv(const aug_var& var, const timeval& now) = 0;

        /**
           Notify of newly accepted connection.

           \return Whether the new connection should be kept.
         */

        virtual bool
        do_accepted(const aug_endpoint& ep, const timeval& now) = 0;

        virtual void
        do_connected(const aug_endpoint& ep, const timeval& now) = 0;

        /**
           Process events.

           \return Whether the connection state changed as a result of
           processing.
         */

        virtual bool
        do_process(unsigned short events, const timeval& now) = 0;

        virtual void
        do_shutdown(unsigned flags, const timeval& now) = 0;

        /**
           Initiate application-level teardown.
         */

        virtual void
        do_teardown(const timeval& now) = 0;

        virtual bool
        do_authcert(const char* subject, const char* issuer) = 0;

        virtual const endpoint&
        do_peername() const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        void
        send(const void* buf, size_t size, const timeval& now)
        {
            do_send(buf, size, now);
        }
        void
        sendv(const aug_var& var, const timeval& now)
        {
            do_sendv(var, now);
        }
        bool
        accepted(const aug_endpoint& ep, const timeval& now)
        {
            return do_accepted(ep, now);
        }
        void
        connected(const aug_endpoint& ep, const timeval& now)
        {
            do_connected(ep, now);
        }
        bool
        process(unsigned short events, const timeval& now)
        {
            return do_process(events, now);
        }
        void
        shutdown(unsigned flags, const timeval& now)
        {
            do_shutdown(flags, now);
        }
        void
        teardown(const timeval& now)
        {
            do_teardown(now);
        }
        bool
        authcert(const char* subject, const char* issuer)
        {
            return do_authcert(subject, issuer);
        }
        const endpoint&
        peername() const
        {
            return do_peername();
        }
    };

    typedef smartptr<conn_base> connptr;

    inline bool
    sendable(const conn_base& conn)
    {
        return conn.state() < SHUTDOWN;
    }

    class connected : public conn_base {

        sessionptr session_;
        augmod_object& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        smartfd sfd_;
        endpoint endpoint_;
        sockstate state_;
        bool close_;

        /**
           Waiting for writability since.
        */

        timeval since_;

        augmod_object&
        do_get();

        const augmod_object&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        void
        do_send(const void* buf, size_t size, const timeval& now);

        void
        do_sendv(const aug_var& var, const timeval& now);

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
        ~connected() AUG_NOTHROW;

        connected(const sessionptr& session, augmod_object& sock,
                  buffer& buffer, rwtimer& rwtimer, const smartfd& sfd,
                  const endpoint& ep, bool close);
    };

    class handshake : public conn_base {

        sessionptr session_;
        augmod_object& sock_;
        buffer& buffer_;
        connector connector_;
        smartfd sfd_;
        endpoint endpoint_;
        sockstate state_;

        augmod_object&
        do_get();

        const augmod_object&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        void
        do_send(const void* buf, size_t size, const timeval& now);

        void
        do_sendv(const aug_var& var, const timeval& now);

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
        ~handshake() AUG_NOTHROW;

        handshake(const sessionptr& session, augmod_object& sock,
                  buffer& buffer, const char* host, const char* port);
    };
}

#endif // AUGRTPP_CONN_HPP