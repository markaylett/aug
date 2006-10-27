/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_MODULE_HPP
#define AUGAS_MODULE_HPP

#include "augas/module.h"

#include "augsyspp/dlfcn.hpp"

namespace augas {

    class module {
        aug::dlib lib_;
        augas_unloadfn unloadfn_;
        augas_module module_;
    public:
        ~module() AUG_NOTHROW;

        module(const char* path, const augas_service& service);

        void
        close(const augas_session& s) const;

        void
        open(augas_session& s, const char* serv) const;

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
}

#endif // AUGAS_MODULE_HPP
