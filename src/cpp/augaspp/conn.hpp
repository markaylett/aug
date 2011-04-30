/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
                const aug_timeval& now) = 0;

        virtual void
        do_sendv(chanref chan, blobref blob, const aug_timeval& now) = 0;

        /**
         * Notify of newly accepted connection.
         *
         * @return Whether the new connection should be kept.
         */

        virtual mod_bool
        do_accepted(const std::string& name, const aug_timeval& now) = 0;

        virtual void
        do_connected(const std::string& name, const aug_timeval& now) = 0;

        virtual mod_bool
        do_auth(const char* subject, const char* issuer) = 0;

        /**
         * Process events.
         */

        virtual void
        do_process_BI(chanref chan, unsigned short events,
                      const aug_timeval& now) = 0;

        /**
         * Initiate application-level teardown.
         */

        virtual void
        do_teardown(const aug_timeval& now) = 0;

        virtual std::string
        do_peername(chanref chan) const = 0;

    public:
        ~conn_base() AUG_NOTHROW;

        void
        send(chanref chan, const void* buf, size_t size,
             const aug_timeval& now)
        {
            do_send(chan, buf, size, now);
        }
        void
        sendv(chanref chan, blobref blob, const aug_timeval& now)
        {
            do_sendv(chan, blob, now);
        }
        mod_bool
        accepted(const std::string& name, const aug_timeval& now)
        {
            return do_accepted(name, now);
        }
        void
        connected(const std::string& name, const aug_timeval& now)
        {
            do_connected(name, now);
        }
        mod_bool
        auth(const char* subject, const char* issuer)
        {
            return do_auth(subject, issuer);
        }
        void
        process_BI(chanref chan, unsigned short events, const aug_timeval& now)
        {
            do_process_BI(chan, events, now);
        }
        void
        teardown(const aug_timeval& now)
        {
            do_teardown(now);
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

    class connimpl : public mpool_ops {

        session_base& session_;
        mod_handle& sock_;
        buffer& buffer_;
        rwtimer& rwtimer_;
        sockstate state_;
        mod_bool accepted_;

        /**
         * Waiting for writability since.
         */

        aug_timeval since_;

    public:
        ~connimpl() AUG_NOTHROW;

        connimpl(session_base& session, mod_handle& sock, buffer& buffer,
                 rwtimer& rwtimer, mod_bool accepted);

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

        void
        send(chanref chan, const void* buf, size_t size,
             const aug_timeval& now);

        void
        sendv(chanref chan, blobref blob, const aug_timeval& now);

        mod_bool
        accepted(const std::string& name, const aug_timeval& now);

        void
        connected(const std::string& name, const aug_timeval& now);

        mod_bool
        auth(const char* subject, const char* issuer);

        void
        process_BI(chanref chan, unsigned short events, const aug_timeval& now);

        void
        error(const char* desc);

        void
        shutdown(chanref chan, unsigned flags, const aug_timeval& now);

        void
        teardown(const aug_timeval& now);

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
