/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_SERV_HPP
#define DAUG_SERV_HPP

#include "daug/module.hpp"

#include "augrtpp/serv.hpp"

namespace augas {

    class serv : public aug::serv_base {

        moduleptr module_;
        augas_serv serv_;
        bool active_;

        augas_serv&
        do_get() AUG_NOTHROW;

        const augas_serv&
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
        do_closed(const augas_object& sock) const AUG_NOTHROW;

        void
        do_teardown(const augas_object& sock) const AUG_NOTHROW;

        bool
        do_accepted(augas_object& sock, const char* addr,
                    unsigned short port) const AUG_NOTHROW;

        void
        do_connected(augas_object& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW;

        void
        do_data(const augas_object& sock, const char* buf,
                size_t size) const AUG_NOTHROW;

        void
        do_rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        do_expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        do_authcert(const augas_object& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW;

    public:
        ~serv() AUG_NOTHROW;

        serv(const moduleptr& module, const char* name);
    };

    const augas_serv*
    getserv();
}

#endif // DAUG_SERV_HPP
