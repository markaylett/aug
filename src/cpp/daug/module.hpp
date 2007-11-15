/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "maud.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include <string>

namespace augas {

    class module {
        const std::string name_;
        aug::dlib lib_;
        maud_termfn termfn_;
        maud_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const maud_host& host,
               void (*teardown)(const maud_object*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(maud_session& session) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW;

        void
        closed(const maud_object& sock) const AUG_NOTHROW;

        void
        teardown(const maud_object& sock) const AUG_NOTHROW;

        bool
        accepted(maud_object& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW;

        void
        connected(maud_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        void
        data(const maud_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const maud_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const maud_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const maud_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const maud_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
