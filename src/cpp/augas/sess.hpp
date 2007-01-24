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
        bool close_;

    public:
        ~sess() AUG_NOTHROW;

        sess(const moduleptr& module, const char* name);

        bool
        open() AUG_NOTHROW
        {
            return close_ = module_->opensess(sess_);
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
        close(const augas_file& file) const AUG_NOTHROW
        {
            module_->close(file);
        }
        bool
        accept(augas_file& file, const char* addr,
               unsigned short port) const AUG_NOTHROW
        {
            return module_->accept(file, addr, port);
        }
        bool
        connect(augas_file& file, const char* addr,
                unsigned short port) const AUG_NOTHROW
        {
            return module_->connect(file, addr, port);
        }
        bool
        data(const augas_file& file, const char* buf,
             size_t size) const AUG_NOTHROW
        {
            return module_->data(file, buf, size);
        }
        bool
        rdexpire(const augas_file& file, unsigned& ms) const AUG_NOTHROW
        {
            return module_->rdexpire(file, ms);
        }
        bool
        wrexpire(const augas_file& file, unsigned& ms) const AUG_NOTHROW
        {
            return module_->wrexpire(file, ms);
        }
        bool
        teardown(const augas_file& file) const AUG_NOTHROW
        {
            return module_->teardown(file);
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
        isopen() const AUG_NOTHROW
        {
            return close_;
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
