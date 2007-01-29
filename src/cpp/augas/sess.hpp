/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SESS_HPP
#define AUGAS_SESS_HPP

#include "augas/module.hpp"

namespace augas {

    class sess {
    public:
        typedef augas_sess ctype;
    private:

        moduleptr module_;
        augas_sess sess_;
        bool active_;

    public:
        ~sess() AUG_NOTHROW;

        sess(const moduleptr& module, const char* name);

        bool
        init() AUG_NOTHROW
        {
            active_ = true; // Functions may be called during initialisation.
            return active_ = module_->init(sess_);
        }
        bool
        event(int type, void* user) const AUG_NOTHROW
        {
            return module_->event(sess_, type, user);
        }
        bool
        expire(unsigned tid, void* user, unsigned& ms) const AUG_NOTHROW
        {
            return module_->expire(sess_, tid, user, ms);
        }
        bool
        reconf() const AUG_NOTHROW
        {
            return module_->reconf(sess_);
        }
        void
        closed(const augas_sock& sock) const AUG_NOTHROW
        {
            module_->closed(sock);
        }
        bool
        accept(augas_sock& sock, const char* addr,
               unsigned short port) const AUG_NOTHROW
        {
            return module_->accept(sock, addr, port);
        }
        bool
        connected(augas_sock& sock, const char* addr,
                  unsigned short port) const AUG_NOTHROW
        {
            return module_->connected(sock, addr, port);
        }
        bool
        data(const augas_sock& sock, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            return module_->data(sock, buf, size);
        }
        bool
        rdexpire(const augas_sock& sock, unsigned& ms) const AUG_NOTHROW
        {
            return module_->rdexpire(sock, ms);
        }
        bool
        wrexpire(const augas_sock& sock, unsigned& ms) const AUG_NOTHROW
        {
            return module_->wrexpire(sock, ms);
        }
        bool
        teardown(const augas_sock& sock) const AUG_NOTHROW
        {
            return module_->teardown(sock);
        }
        operator augas_sess&() AUG_NOTHROW
        {
            return sess_;
        }
        operator const augas_sess&() const AUG_NOTHROW
        {
            return sess_;
        }
        bool
        active() const AUG_NOTHROW
        {
            return active_;
        }
        const char*
        name() const AUG_NOTHROW
        {
            return sess_.name_;
        }
    };

    typedef aug::smartptr<sess> sessptr;
}

#endif // AUGAS_SESS_HPP
