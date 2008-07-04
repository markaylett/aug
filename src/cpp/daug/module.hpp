/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "augmod.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include "augabipp.hpp"

#include <string>

namespace daug {

    class module {
        const std::string name_;
        aug::dlib lib_;
        mod_termfn termfn_;
        mod_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const mod_host& host,
               void (*teardown)(const mod_handle*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(mod_session& session) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type,
              aug::objectref ob) const AUG_NOTHROW;

        void
        closed(const mod_handle& sock) const AUG_NOTHROW;

        void
        teardown(const mod_handle& sock) const AUG_NOTHROW;

        bool
        accepted(mod_handle& sock, const char* name) const AUG_NOTHROW;

        void
        connected(mod_handle& sock, const char* name) const AUG_NOTHROW;

        void
        data(const mod_handle& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const mod_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const mod_handle& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const mod_handle& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
