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
#ifndef AUGD_SESSION_HPP
#define AUGD_SESSION_HPP

#include "augd/module.hpp"

#include "augaspp/session.hpp"

namespace augd {

    class session : public aug::session_base, public aug::mpool_ops {

        char name_[MOD_MAXNAME + 1];
        moduleptr module_;
        mod::sessionptr session_;
        mod_bool active_;

        const char*
        do_name() const AUG_NOTHROW;

        mod_bool
        do_active() const AUG_NOTHROW;

        mod_bool
        do_start() AUG_NOTHROW;

        void
        do_reconf() const AUG_NOTHROW;

        void
        do_event(const char* from, const char* type, mod_id id,
                 aug::objectref ob) const AUG_NOTHROW;

        void
        do_closed(mod_handle& sock) const AUG_NOTHROW;

        void
        do_teardown(mod_handle& sock) const AUG_NOTHROW;

        mod_bool
        do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW;

        void
        do_connected(mod_handle& sock, const char* name) const AUG_NOTHROW;

        mod_bool
        do_auth(mod_handle& sock, const char* subject,
                const char* issuer) const AUG_NOTHROW;

        void
        do_recv(mod_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_error(mod_handle& sock, const char* desc) const AUG_NOTHROW;

        void
        do_rdexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(mod_handle& timer, unsigned& ms) const AUG_NOTHROW;

    public:
        ~session() AUG_NOTHROW;

        session(const char* name, const moduleptr& module);
    };

    const aug::session_base&
    getsession();
}

#endif // AUGD_SESSION_HPP
