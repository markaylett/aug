/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "augrt.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include <string>

namespace augrt {

    class module {
        const std::string name_;
        aug::dlib lib_;
        augrt_termfn termfn_;
        augrt_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const augrt_host& host, void (*teardown)(const augrt_object*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(augrt_session& session) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW;

        void
        closed(const augrt_object& sock) const AUG_NOTHROW;

        void
        teardown(const augrt_object& sock) const AUG_NOTHROW;

        bool
        accepted(augrt_object& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW;

        void
        connected(augrt_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        void
        data(const augrt_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const augrt_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const augrt_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const augrt_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
