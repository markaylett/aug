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
        do_send(const void* buf, size_t size, const timeval& now) = 0;

        virtual void
        do_sendv(blobref ref, const timeval& now) = 0;

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
         *
         * @return Whether the connection state changed as a result of
         * processing.
         */

        virtual bool
        do_process(obref<aug_stream> stream, unsigned short events,
                   const timeval& now) = 0;

        virtual void
        do_shutdown(unsigned flags, const timeval& now) = 0;

        /**
         * Initiate application-level teardown.
         */

        virtual void
        do_teardown(const timeval& now) = 0;

        virtual bool
        do_authcert(const char* subject, const char* issuer) = 0;

        virtual std::string
        do_peername() const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        void
        send(const void* buf, size_t size, const timeval& now)
        {
            do_send(buf, size, now);
        }
        void
        sendv(blobref ref, const timeval& now)
        {
            do_sendv(ref, now);
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
        bool
        process(obref<aug_stream> stream, unsigned short events,
                const timeval& now)
        {
            return do_process(stream, events, now);
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
        std::string
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
        mod_handle& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        chanptr chan_;
        sockstate state_;
        bool close_;

        /**
         * Waiting for writability since.
         */

        timeval since_;

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
        ~connected() AUG_NOTHROW;

        connected(const sessionptr& session, mod_handle& sock,
                  buffer& buffer, rwtimer& rwtimer,
                  const chanptr& chan, bool close);
    };
}

#endif // AUGASPP_CONN_HPP
