/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SERV_HPP
#define AUGRTPP_SERV_HPP

#include "augrtpp/config.hpp"

#include "augsyspp/smartptr.hpp"

#include "augas.h"

namespace aug {

    class serv_base {
    public:
        typedef augas_serv ctype;
    private:

        virtual augas_serv&
        do_get() AUG_NOTHROW = 0;

        virtual const augas_serv&
        do_get() const AUG_NOTHROW = 0;

        virtual bool
        do_active() const AUG_NOTHROW = 0;

        virtual bool
        do_start() AUG_NOTHROW = 0;

        virtual void
        do_reconf() const AUG_NOTHROW = 0;

        virtual void
        do_event(const char* from, const char* type, const void* user,
                 size_t size) const AUG_NOTHROW = 0;

        virtual void
        do_closed(const augas_object& sock) const AUG_NOTHROW = 0;

        virtual void
        do_teardown(const augas_object& sock) const AUG_NOTHROW = 0;

        virtual bool
        do_accept(augas_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW = 0;

        virtual void
        do_connected(augas_object& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW = 0;

        virtual void
        do_data(const augas_object& sock, const char* buf,
                size_t size) const AUG_NOTHROW = 0;

        virtual void
        do_rdexpire(const augas_object& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_wrexpire(const augas_object& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_expire(const augas_object& timer,
                  unsigned& ms) const AUG_NOTHROW = 0;

        virtual bool
        do_authcert(const augas_object& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW = 0;

    public:
        virtual
        ~serv_base() AUG_NOTHROW;

        augas_serv&
        get() AUG_NOTHROW
        {
            return do_get();
        }
        const augas_serv&
        get() const AUG_NOTHROW
        {
            return do_get();
        }
        bool
        active() const AUG_NOTHROW
        {
            return do_active();
        }
        bool
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
        event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW
        {
            do_event(from, type, user, size);
        }
        void
        closed(const augas_object& sock) const AUG_NOTHROW
        {
            do_closed(sock);
        }
        void
        teardown(const augas_object& sock) const AUG_NOTHROW
        {
            do_teardown(sock);
        }
        bool
        accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW
        {
            return do_accept(sock, addr, port);
        }
        void
        connected(augas_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
        {
            do_connected(sock, addr, port);
        }
        void
        data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            do_data(sock, buf, size);
        }
        void
        rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW
        {
            do_expire(timer, ms);
        }
        bool
        authcert(const augas_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
        {
            return do_authcert(sock, subject, issuer);
        }
        operator augas_serv&() AUG_NOTHROW
        {
            return do_get();
        }
        operator const augas_serv&() const AUG_NOTHROW
        {
            return do_get();
        }
    };

    typedef smartptr<serv_base> servptr;
}

#endif // AUGRTPP_SERV_HPP
