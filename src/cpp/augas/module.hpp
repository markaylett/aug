/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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
        const std::string modname_;
        aug::dlib lib_;
        augas_unloadfn unloadfn_;
        augas_module module_;

        module(const module&);

        module&
        operator =(const module&);

    public:
        ~module() AUG_NOTHROW;

        module(const std::string& modname, const char* path,
               const augas_host& host);

        void
        close(const augas_session& s) const;

        void
        open(augas_session& s, const char* serv, const char* peer) const;

        void
        data(const augas_session& s, const char* buf,
             size_t size) const;

        void
        rdexpire(const augas_session& s, unsigned& ms) const;

        void
        wrexpire(const augas_session& s, unsigned& ms) const;

        void
        stop(const augas_session& s) const;

        void
        event(int type, void* arg) const;

        void
        expire(void* arg, unsigned id, unsigned* ms) const;

        void
        reconf() const;
    };

    typedef aug::smartptr<module> moduleptr;
}

#endif // AUGAS_MODULE_HPP
