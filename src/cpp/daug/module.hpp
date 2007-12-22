/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "aum.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include "aubpp.hpp"

#include <string>

namespace daug {

    class module {
        const std::string name_;
        aug::dlib lib_;
        aum_termfn termfn_;
        aum_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const aum_host& host,
               void (*teardown)(const aum_handle*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(aum_session& session) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type,
              aub::objectref ob) const AUG_NOTHROW;

        void
        closed(const aum_handle& sock) const AUG_NOTHROW;

        void
        teardown(const aum_handle& sock) const AUG_NOTHROW;

        bool
        accepted(aum_handle& sock, const char* addr,
                 unsigned short port) const AUG_NOTHROW;

        void
        connected(aum_handle& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        void
        data(const aum_handle& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const aum_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const aum_handle& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const aum_handle& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const aum_handle& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
