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
#ifndef AUGASPP_SESSION_HPP
#define AUGASPP_SESSION_HPP

#include "augaspp/config.hpp"

#include "augsyspp/smartptr.hpp"

#include "augctxpp/mpool.hpp"

#include "augabipp.hpp"

#include "augmod.h"

namespace aug {

    class AUGASPP_API session_base {

        virtual const char*
        do_name() const AUG_NOTHROW = 0;

        virtual mod_bool
        do_active() const AUG_NOTHROW = 0;

        // Sessions should be active during calls to start().  If start()
        // fails they should be made inactive.

        virtual mod_bool
        do_start() AUG_NOTHROW = 0;

        virtual void
        do_reconf() const AUG_NOTHROW = 0;

        virtual void
        do_event(const char* from, const char* type, mod_id id,
                 objectref ob) const AUG_NOTHROW = 0;

        virtual void
        do_closed(mod_handle& sock) const AUG_NOTHROW = 0;

        virtual void
        do_teardown(mod_handle& sock) const AUG_NOTHROW = 0;

        virtual mod_bool
        do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW = 0;

        virtual void
        do_connected(mod_handle& sock,
                     const char* name) const AUG_NOTHROW = 0;

        virtual mod_bool
        do_auth(mod_handle& sock, const char* subject,
                const char* issuer) const AUG_NOTHROW = 0;

        virtual void
        do_recv(mod_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW = 0;

        virtual void
        do_mrecv(const char* node, unsigned sess, unsigned short type,
                 const void* buf, size_t len) const AUG_NOTHROW = 0;

        virtual void
        do_error(mod_handle& sock, const char* desc) const AUG_NOTHROW = 0;

        virtual void
        do_rdexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_wrexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_expire(mod_handle& timer, unsigned& ms) const AUG_NOTHROW = 0;

    public:
        virtual
        ~session_base() AUG_NOTHROW;

        const char*
        name() const AUG_NOTHROW
        {
            return do_name();
        }
        mod_bool
        active() const AUG_NOTHROW
        {
            return do_active();
        }
        mod_bool
        start() AUG_NOTHROW
        {
            return do_start();
        }
        void
        reconf() const AUG_NOTHROW
        {
            return do_reconf();
        }
        void
        event(const char* from, const char* type, mod_id id,
              objectref ob) const AUG_NOTHROW
        {
            do_event(from, type, id, ob);
        }
        void
        closed(mod_handle& sock) const AUG_NOTHROW
        {
            do_closed(sock);
        }
        void
        teardown(mod_handle& sock) const AUG_NOTHROW
        {
            do_teardown(sock);
        }
        mod_bool
        accepted(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
            return do_accepted(sock, name);
        }
        void
        connected(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
            do_connected(sock, name);
        }
        mod_bool
        auth(mod_handle& sock, const char* subject,
             const char* issuer) const AUG_NOTHROW
        {
            return do_auth(sock, subject, issuer);
        }
        void
        recv(mod_handle& sock, const char* buf, size_t size) const AUG_NOTHROW
        {
            do_recv(sock, buf, size);
        }
        void
        mrecv(const char* node, unsigned sess, unsigned short type,
              const void* buf, size_t len) const AUG_NOTHROW
        {
            do_mrecv(node, sess, type, buf, len);
        }
        void
        error(mod_handle& sock, const char* desc) const AUG_NOTHROW
        {
            do_error(sock, desc);
        }
        void
        rdexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(mod_handle& timer, unsigned& ms) const AUG_NOTHROW
        {
            do_expire(timer, ms);
        }
    };

    typedef smartptr<session_base> sessionptr;
}

#endif // AUGASPP_SESSION_HPP
