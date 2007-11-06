/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "augmod.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include <string>

namespace augrt {

    class module {
        const std::string name_;
        aug::dlib lib_;
        augmod_termfn termfn_;
        augmod_control control_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const augmod_host& host,
               void (*teardown)(const augmod_object*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(augmod_session& session) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW;

        void
        closed(const augmod_object& sock) const AUG_NOTHROW;

        void
        teardown(const augmod_object& sock) const AUG_NOTHROW;

        bool
        accepted(augmod_object& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW;

        void
        connected(augmod_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        void
        data(const augmod_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const augmod_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const augmod_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const augmod_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const augmod_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
