/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SESSION_HPP
#define DAUG_SESSION_HPP

#include "daug/module.hpp"

#include "augaspp/session.hpp"

namespace daug {

    class session : public aug::session_base {

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
