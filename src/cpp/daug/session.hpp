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
#ifndef DAUG_SESSION_HPP
#define DAUG_SESSION_HPP

#include "daug/module.hpp"

#include "augaspp/session.hpp"

namespace daug {

    class session : public aug::session_base, public aug::mpool_ops {

        moduleptr module_;
        mod_session session_;
        bool active_;

        mod_session&
        do_get() AUG_NOTHROW;

        const mod_session&
        do_get() const AUG_NOTHROW;

        bool
        do_active() const AUG_NOTHROW;

        bool
        do_start() AUG_NOTHROW;

        void
        do_reconf() const AUG_NOTHROW;

        void
        do_event(const char* from, const char* type,
                 aug::objectref ob) const AUG_NOTHROW;

        void
        do_closed(const mod_handle& sock) const AUG_NOTHROW;

        void
        do_teardown(const mod_handle& sock) const AUG_NOTHROW;

        bool
        do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW;

        void
        do_connected(mod_handle& sock, const char* name) const AUG_NOTHROW;

        bool
        do_auth(const mod_handle& sock, const char* subject,
                const char* issuer) const AUG_NOTHROW;

        void
        do_recv(const mod_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_error(const mod_handle& sock, const char* desc) const AUG_NOTHROW;

        void
        do_rdexpire(const mod_handle& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(const mod_handle& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(const mod_handle& timer, unsigned& ms) const AUG_NOTHROW;

    public:
        ~session() AUG_NOTHROW;

        session(const moduleptr& module, const char* name);
    };

    const mod_session*
    getsession();
}

#endif // DAUG_SESSION_HPP
