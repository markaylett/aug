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
        augrt_session session_;
        bool active_;

        augrt_session&
        do_get() AUG_NOTHROW;

        const augrt_session&
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
        do_closed(const augrt_object& sock) const AUG_NOTHROW;

        void
        do_teardown(const augrt_object& sock) const AUG_NOTHROW;

        bool
        do_accepted(augrt_object& sock, const char* addr,
                    unsigned short port) const AUG_NOTHROW;

        void
        do_connected(augrt_object& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW;

        void
        do_data(const augrt_object& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_rdexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(const augrt_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        do_authcert(const augrt_object& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW;

    public:
        ~session() AUG_NOTHROW;

        session(const moduleptr& module, const char* name);
    };

    const augrt_session*
    getsession();
}

#endif // DAUG_SESSION_HPP
