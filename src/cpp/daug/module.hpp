/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef DAUG_MODULE_HPP
#define DAUG_MODULE_HPP

#include "augas.h"

#include "augsyspp/dlfcn.hpp"
#include "augsyspp/smartptr.hpp"

#include <string>

namespace augas {

    class module {
        const std::string name_;
        aug::dlib lib_;
        augas_termfn termfn_;
        augas_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& name, const char* path,
               const augas_host& host, void (*teardown)(const augas_object*));

        void
        stop() const AUG_NOTHROW;

        bool
        start(augas_serv& serv) const AUG_NOTHROW;

        void
        reconf() const AUG_NOTHROW;

        void
        event(const char* from, const char* type, const void* user,
              size_t size) const AUG_NOTHROW;

        void
        closed(const augas_object& sock) const AUG_NOTHROW;

        void
        teardown(const augas_object& sock) const AUG_NOTHROW;

        bool
        accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW;

        void
        connected(augas_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        void
        data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        void
        rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        void
        expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW;

        bool
        authcert(const augas_object& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // DAUG_MODULE_HPP
