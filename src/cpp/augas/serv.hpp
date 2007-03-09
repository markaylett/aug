/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SERV_HPP
#define AUGAS_SERV_HPP

#include "augas/module.hpp"

namespace augas {

    class serv {
    public:
        typedef augas_serv ctype;
    private:

        moduleptr module_;
        augas_serv serv_;
        bool active_;

    public:
        ~serv() AUG_NOTHROW;

        serv(const moduleptr& module, const char* name);

        bool
        start() AUG_NOTHROW;

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

        operator augas_serv&() AUG_NOTHROW
        {
            return serv_;
        }
        operator const augas_serv&() const AUG_NOTHROW
        {
            return serv_;
        }
        bool
        active() const AUG_NOTHROW
        {
            return active_;
        }
        const char*
        name() const AUG_NOTHROW
        {
            return serv_.name_;
        }
    };

    typedef aug::smartptr<serv> servptr;

    const augas_serv*
    getserv();
}

#endif // AUGAS_SERV_HPP
