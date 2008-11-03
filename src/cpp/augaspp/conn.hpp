/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_CONN_HPP
#define AUGASPP_CONN_HPP

#include "augaspp/rwtimer.hpp"
#include "augaspp/sock.hpp"

#include "augnetpp.hpp"

namespace aug {

    class buffer;

    class conn_base : public sock_base {

        virtual void
        do_send(chanref chan, const void* buf, size_t size,
                const timeval& now) = 0;

        virtual void
        do_sendv(chanref chan, blobref blob, const timeval& now) = 0;

        /**
         * Notify of newly accepted connection.
         *
         * @return Whether the new connection should be kept.
         */

        virtual bool
        do_accepted(const std::string& name, const timeval& now) = 0;

        virtual void
        do_connected(const std::string& name, const timeval& now) = 0;

        /**
         * Process events.
         */

        virtual void
        do_process(chanref chan, unsigned short events,
                   const timeval& now) = 0;

        /**
         * Initiate application-level teardown.
         */

        virtual void
        do_teardown(const timeval& now) = 0;

        virtual bool
        do_authcert(const char* subject, const char* issuer) = 0;

        virtual std::string
        do_peername(chanref chan) const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        void
        send(chanref chan, const void* buf, size_t size, const timeval& now)
        {
            do_send(chan, buf, size, now);
        }
        void
        sendv(chanref chan, blobref blob, const timeval& now)
        {
            do_sendv(chan, blob, now);
        }
        bool
        accepted(const std::string& name, const timeval& now)
        {
            return do_accepted(name, now);
        }
        void
        connected(const std::string& name, const timeval& now)
        {
            do_connected(name, now);
        }
        void
        process(chanref chan, unsigned short events, const timeval& now)
        {
            do_process(chan, events, now);
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
        std::string
        peername(chanref chan) const
        {
            return do_peername(chan);
        }
    };

    typedef smartptr<conn_base> connptr;

    inline bool
    sendable(const conn_base& conn)
    {
        return conn.state() < SHUTDOWN;
    }

    class connimpl {

        sessionptr session_;
        mod_handle& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        sockstate state_;
        bool accepted_;

        /**
         * Waiting for writability since.
         */

        timeval since_;

    public:
        ~connimpl() AUG_NOTHROW;

        connimpl(const sessionptr& session, mod_handle& sock, buffer& buffer,
                 rwtimer& rwtimer, bool accepted);

        mod_handle&
        get()
        {
            return sock_;
        }

        const mod_handle&
        get() const
        {
            return sock_;
        }

        const sessionptr&
        session() const
        {
            return session_;
        }

        void
        send(chanref chan, const void* buf, size_t size, const timeval& now);

        void
        sendv(chanref chan, blobref blob, const timeval& now);

        bool
        accepted(const std::string& name, const timeval& now);

        void
        connected(const std::string& name, const timeval& now);

        void
        process(chanref chan, unsigned short events, const timeval& now);

        void
        error(const char* desc);

        void
        shutdown(chanref chan, unsigned flags, const timeval& now);

        void
        teardown(const timeval& now);

        bool
        authcert(const char* subject, const char* issuer);

        std::string
        peername(chanref chan) const;

        sockstate
        state() const
        {
            return state_;
        }
    };
}

#endif // AUGASPP_CONN_HPP
