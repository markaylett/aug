/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_CONN_HPP
#define AUGRTPP_CONN_HPP

#include "augrtpp/rwtimer.hpp"
#include "augrtpp/sock.hpp"

#include "augnetpp.hpp"

namespace aug {

    class buffer;

    class conn_base : public sock_base {

        virtual void
        do_send(const void* buf, size_t size) = 0;

        virtual void
        do_sendv(const aug_var& var) = 0;

        virtual bool
        do_accepted(const aug_endpoint& ep) = 0;

        virtual void
        do_connected(const aug_endpoint& ep) = 0;

        virtual bool
        do_process(unsigned short events) = 0;

        virtual void
        do_shutdown() = 0;

        virtual void
        do_teardown() = 0;

        virtual bool
        do_authcert(const char* subject, const char* issuer) = 0;

        virtual const endpoint&
        do_peername() const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        void
        send(const void* buf, size_t size)
        {
            do_send(buf, size);
        }
        void
        sendv(const aug_var& var)
        {
            do_sendv(var);
        }
        bool
        accepted(const aug_endpoint& ep)
        {
            return do_accepted(ep);
        }
        void
        connected(const aug_endpoint& ep)
        {
            do_connected(ep);
        }
        bool
        process(unsigned short events)
        {
            return do_process(events);
        }
        void
        shutdown()
        {
            do_shutdown();
        }
        void
        teardown()
        {
            do_teardown();
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
        augrt_object& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        smartfd sfd_;
        endpoint endpoint_;
        sockstate state_;
        bool close_;

        augrt_object&
        do_get();

        const augrt_object&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        void
        do_send(const void* buf, size_t size);

        void
        do_sendv(const aug_var& var);

        bool
        do_accepted(const aug_endpoint& ep);

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

        const endpoint&
        do_peername() const;

        sockstate
        do_state() const;

    public:
        ~connected() AUG_NOTHROW;

        connected(const sessionptr& session, augrt_object& sock,
                  buffer& buffer, rwtimer& rwtimer, const smartfd& sfd,
                  const endpoint& ep, bool close);
    };

    class handshake : public conn_base {

        sessionptr session_;
        augrt_object& sock_;
        buffer& buffer_;
        connector connector_;
        smartfd sfd_;
        endpoint endpoint_;
        sockstate state_;

        augrt_object&
        do_get();

        const augrt_object&
        do_get() const;

        const sessionptr&
        do_session() const;

        smartfd
        do_sfd() const;

        void
        do_send(const void* buf, size_t size);

        void
        do_sendv(const aug_var& var);

        bool
        do_accepted(const aug_endpoint& ep);

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

        const endpoint&
        do_peername() const;

        sockstate
        do_state() const;

    public:
        ~handshake() AUG_NOTHROW;

        handshake(const sessionptr& session, augrt_object& sock,
                  buffer& buffer, const char* host, const char* port);
    };
}

#endif // AUGRTPP_CONN_HPP
