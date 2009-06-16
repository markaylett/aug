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
#ifndef AUGASPP_CLNTCONN_HPP
#define AUGASPP_CLNTCONN_HPP

#include "augaspp/buffer.hpp"
#include "augaspp/conn.hpp"

namespace aug {

    class clntconn : public conn_base, public rwtimer_base, public mpool_ops {

        sessionptr session_;
        handle sock_;
        buffer buffer_;
        rwtimer rwtimer_;
        aug::connimpl impl_;

        // object_base.

        mod_handle&
        do_get();

        const mod_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        // sock_base.

        void
        do_error(const char* desc);

        void
        do_shutdown(chanref chan, unsigned flags, const aug_timeval& now);

        sockstate
        do_state() const;

        // conn_base.

        void
        do_send(chanref chan, const void* buf, size_t size,
                const aug_timeval& now);

        void
        do_sendv(chanref chan, blobref blob, const aug_timeval& now);

        mod_bool
        do_accepted(const std::string& name, const aug_timeval& now);

        void
        do_connected(const std::string& name, const aug_timeval& now);

        mod_bool
        do_auth(const char* subject, const char* issuer);

        void
        do_process(chanref chan, unsigned short events,
                   const aug_timeval& now);

        void
        do_teardown(const aug_timeval& now);

        std::string
        do_peername(chanref chan) const;

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

    public:
        ~clntconn() AUG_NOTHROW;

        clntconn(mpoolref mpool, const sessionptr& session,
                 aug_timers_t timers, aug_id id, objectref ob);
    };
}

#endif // AUGASPP_CLNTCONN_HPP
