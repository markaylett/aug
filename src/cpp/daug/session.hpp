/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SESSION_HPP
#define DAUG_SESSION_HPP

#include "daug/module.hpp"

#include "augrtpp/session.hpp"

namespace augrt {

    class session : public aug::session_base {

        moduleptr module_;
        augmod_session session_;
        bool active_;

        augmod_session&
        do_get() AUG_NOTHROW;

        const augmod_session&
        do_get() const AUG_NOTHROW;

        bool
        do_active() const AUG_NOTHROW;

        bool
        do_start() AUG_NOTHROW;

        void
        do_reconf() const AUG_NOTHROW;

        void
        do_event(const char* from, const char* type, const void* user,
                 size_t size) const AUG_NOTHROW;

        void
        do_closed(const augmod_object& sock) const AUG_NOTHROW;

        void
        do_teardown(const augmod_object& sock) const AUG_NOTHROW;

        bool
        do_accepted(augmod_object& sock, const char* addr,
                    unsigned short port) const AUG_NOTHROW;

        void
        do_connected(augmod_object& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW;

        void
        do_data(const augmod_object& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_rdexpire(const augmod_object& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(const augmod_object& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(const augmod_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        do_authcert(const augmod_object& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW;

    public:
        ~session() AUG_NOTHROW;

        session(const moduleptr& module, const char* name);
    };

    const augmod_session*
    getsession();
}

#endif // DAUG_SESSION_HPP
