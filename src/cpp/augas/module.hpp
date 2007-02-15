/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MODULE_HPP
#define AUGAS_MODULE_HPP

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
               const augas_host& host);

        void
        destroy(const augas_serv& serv) const AUG_NOTHROW;

        bool
        create(augas_serv& serv) const AUG_NOTHROW;

        bool
        reconf(const augas_serv& serv) const AUG_NOTHROW;

        bool
        event(const augas_serv& serv, const char* from,
              const augas_event& event) const AUG_NOTHROW;

        void
        closed(const augas_object& sock) const AUG_NOTHROW;

        bool
        teardown(const augas_object& sock) const AUG_NOTHROW;

        bool
        accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW;

        bool
        connected(augas_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW;

        bool
        data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW;

        bool
        rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        bool
        wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW;

        bool
        expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // AUGAS_MODULE_HPP
