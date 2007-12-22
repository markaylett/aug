/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_SESSION_HPP
#define AUGRTPP_SESSION_HPP

#include "augaspp/config.hpp"

#include "augsyspp/smartptr.hpp"

#include "aubpp.hpp"

#include "aum.h"

namespace aug {

    class AUGRTPP_API session_base {
    public:
        typedef aum_session ctype;
    private:

        virtual aum_session&
        do_get() AUG_NOTHROW = 0;

        virtual const aum_session&
        do_get() const AUG_NOTHROW = 0;

        virtual bool
        do_active() const AUG_NOTHROW = 0;

        // Sessions should be active during calls to start().  If start()
        // fails they should be made inactive.

        virtual bool
        do_start() AUG_NOTHROW = 0;

        virtual void
        do_reconf() const AUG_NOTHROW = 0;

        virtual void
        do_event(const char* from, const char* type,
                 aub::objectref ob) const AUG_NOTHROW = 0;

        virtual void
        do_closed(const aum_handle& sock) const AUG_NOTHROW = 0;

        virtual void
        do_teardown(const aum_handle& sock) const AUG_NOTHROW = 0;

        virtual bool
        do_accepted(aum_handle& sock, const char* addr,
                    unsigned short port) const AUG_NOTHROW = 0;

        virtual void
        do_connected(aum_handle& sock, const char* addr,
                     unsigned short port) const AUG_NOTHROW = 0;

        virtual void
        do_data(const aum_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW = 0;

        virtual void
        do_rdexpire(const aum_handle& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_wrexpire(const aum_handle& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_expire(const aum_handle& timer,
                  unsigned& ms) const AUG_NOTHROW = 0;

        virtual bool
        do_authcert(const aum_handle& sock, const char* subject,
                    const char* issuer) const AUG_NOTHROW = 0;

    public:
        virtual
        ~session_base() AUG_NOTHROW;

        aum_session&
        get() AUG_NOTHROW
        {
            return do_get();
        }
        const aum_session&
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
        event(const char* from, const char* type,
              aub::objectref ob) const AUG_NOTHROW
        {
            do_event(from, type, ob);
        }
        void
        closed(const aum_handle& sock) const AUG_NOTHROW
        {
            do_closed(sock);
        }
        void
        teardown(const aum_handle& sock) const AUG_NOTHROW
        {
            do_teardown(sock);
        }
        bool
        accepted(aum_handle& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW
        {
            return do_accepted(sock, addr, port);
        }
        void
        connected(aum_handle& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
        {
            do_connected(sock, addr, port);
        }
        void
        data(const aum_handle& sock, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            do_data(sock, buf, size);
        }
        void
        rdexpire(const aum_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(const aum_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(const aum_handle& timer, unsigned& ms) const AUG_NOTHROW
        {
            do_expire(timer, ms);
        }
        bool
        authcert(const aum_handle& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
        {
            return do_authcert(sock, subject, issuer);
        }
        operator aum_session&() AUG_NOTHROW
        {
            return do_get();
        }
        operator const aum_session&() const AUG_NOTHROW
        {
            return do_get();
        }
    };

    typedef smartptr<session_base> sessionptr;
}

#endif // AUGRTPP_SESSION_HPP
