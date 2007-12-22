/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SESSION_HPP
#define DAUG_SESSION_HPP

#include "daug/module.hpp"

#include "augaspp/session.hpp"

namespace daug {

    class session : public aug::session_base {

        moduleptr module_;
        aum_session session_;
        bool active_;

        aum_session&
        do_get() AUG_NOTHROW;

        const aum_session&
        do_get() const AUG_NOTHROW;

        bool
        do_active() const AUG_NOTHROW;

        bool
        do_start() AUG_NOTHROW;

        void
        do_reconf() const AUG_NOTHROW;

        void
        do_event(const char* from, const char* type,
                 aub::objectref ob) const AUG_NOTHROW;

        void
        do_closed(const aum_handle& sock) const AUG_NOTHROW;

        void
        do_teardown(const aum_handle& sock) const AUG_NOTHROW;

        bool
        do_accepted(aum_handle& sock, const char* addr,
                    unsigned short port) const AUG_NOTHROW;

        void
        do_connected(aum_handle& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW;

        void
        do_data(const aum_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_rdexpire(const aum_handle& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(const aum_handle& sock,
                    unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(const aum_handle& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        do_authcert(const aum_handle& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW;

    public:
        ~session() AUG_NOTHROW;

        session(const moduleptr& module, const char* name);
    };

    const aum_session*
    getsession();
}

#endif // DAUG_SESSION_HPP
