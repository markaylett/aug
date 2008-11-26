/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
    public:
        typedef mod_session ctype;
    private:

        virtual mod_session&
        do_get() AUG_NOTHROW = 0;

        virtual const mod_session&
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
                 aug::objectref ob) const AUG_NOTHROW = 0;

        virtual void
        do_closed(const mod_handle& sock) const AUG_NOTHROW = 0;

        virtual void
        do_teardown(const mod_handle& sock) const AUG_NOTHROW = 0;

        virtual bool
        do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW = 0;

        virtual void
        do_connected(mod_handle& sock,
                     const char* name) const AUG_NOTHROW = 0;

        virtual bool
        do_auth(const mod_handle& sock, const char* subject,
                const char* issuer) const AUG_NOTHROW = 0;

        virtual void
        do_recv(const mod_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW = 0;

        virtual void
        do_error(const mod_handle& sock,
                 const char* desc) const AUG_NOTHROW = 0;

        virtual void
        do_rdexpire(const mod_handle& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_wrexpire(const mod_handle& sock,
                    unsigned& ms) const AUG_NOTHROW = 0;

        virtual void
        do_expire(const mod_handle& timer,
                  unsigned& ms) const AUG_NOTHROW = 0;

    public:
        virtual
        ~session_base() AUG_NOTHROW;

        mod_session&
        get() AUG_NOTHROW
        {
            return do_get();
        }
        const mod_session&
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
              aug::objectref ob) const AUG_NOTHROW
        {
            do_event(from, type, ob);
        }
        void
        closed(const mod_handle& sock) const AUG_NOTHROW
        {
            do_closed(sock);
        }
        void
        teardown(const mod_handle& sock) const AUG_NOTHROW
        {
            do_teardown(sock);
        }
        bool
        accepted(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
            return do_accepted(sock, name);
        }
        void
        connected(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
            do_connected(sock, name);
        }
        bool
        auth(const mod_handle& sock, const char* subject,
             const char* issuer) const AUG_NOTHROW
        {
            return do_auth(sock, subject, issuer);
        }
        void
        recv(const mod_handle& sock, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            do_recv(sock, buf, size);
        }
        void
        error(const mod_handle& sock, const char* desc) const AUG_NOTHROW
        {
            do_error(sock, desc);
        }
        void
        rdexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_rdexpire(sock, ms);
        }
        void
        wrexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
            do_wrexpire(sock, ms);
        }
        void
        expire(const mod_handle& timer, unsigned& ms) const AUG_NOTHROW
        {
            do_expire(timer, ms);
        }
        operator mod_session&() AUG_NOTHROW
        {
            return do_get();
        }
        operator const mod_session&() const AUG_NOTHROW
        {
            return do_get();
        }
    };

    typedef smartptr<session_base> sessionptr;
}

#endif // AUGASPP_SESSION_HPP
