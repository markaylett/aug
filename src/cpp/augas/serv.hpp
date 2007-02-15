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
        start() AUG_NOTHROW
        {
            active_ = true; // Functions may be called during initialisation.
            return active_ = module_->start(serv_);
        }
        bool
        reconf() const AUG_NOTHROW
        {
            return module_->reconf(serv_);
        }
        bool
        event(const char* from, const augas_event& event) const AUG_NOTHROW
        {
            return module_->event(serv_, from, event);
        }
        void
        closed(const augas_object& sock) const AUG_NOTHROW
        {
            module_->closed(sock);
        }
        bool
        teardown(const augas_object& sock) const AUG_NOTHROW
        {
            return module_->teardown(sock);
        }
        bool
        accept(augas_object& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW
        {
            return module_->accept(sock, addr, port);
        }
        bool
        connected(augas_object& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
        {
            return module_->connected(sock, addr, port);
        }
        bool
        data(const augas_object& sock, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            return module_->data(sock, buf, size);
        }
        bool
        rdexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
        {
            return module_->rdexpire(sock, ms);
        }
        bool
        wrexpire(const augas_object& sock, unsigned& ms) const AUG_NOTHROW
        {
            return module_->wrexpire(sock, ms);
        }
        bool
        expire(const augas_object& timer, unsigned& ms) const AUG_NOTHROW
        {
            return module_->expire(timer, ms);
        }
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
}

#endif // AUGAS_SERV_HPP
