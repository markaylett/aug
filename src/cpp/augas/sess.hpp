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
        bool open_;

    public:
        ~sess() AUG_NOTHROW;

        sess(const moduleptr& module, const char* name);

        void
        open();

        void
        event(int type, void* user) const
        {
            module_->event(sess_, type, user);
        }
        void
        expire(unsigned tid, void* user, unsigned& ms) const
        {
            module_->expire(sess_, tid, user, ms);
        }
        void
        reconf() const
        {
            module_->reconf(sess_);
        }
        void
        close(const augas_file& file) const
        {
            module_->close(file);
        }
        void
        accept(augas_file& file, const char* addr, unsigned short port) const
        {
            module_->accept(file, addr, port);
        }
        void
        connect(augas_file& file, const char* addr, unsigned short port) const
        {
            module_->connect(file, addr, port);
        }
        void
        data(const augas_file& file, const char* buf, size_t size) const
        {
            module_->data(file, buf, size);
        }
        void
        rdexpire(const augas_file& file, unsigned& ms) const
        {
            module_->rdexpire(file, ms);
        }
        void
        wrexpire(const augas_file& file, unsigned& ms) const
        {
            module_->wrexpire(file, ms);
        }
        void
        teardown(const augas_file& file) const
        {
            module_->teardown(file);
        }
        operator augas_sess&()
        {
            return sess_;
        }
        operator const augas_sess&() const
        {
            return sess_;
        }
        bool
        isopen() const
        {
            return open_;
        }
        const char*
        name() const
        {
            return sess_.name_;
        }
    };

    typedef aug::smartptr<sess, aug::scoped_lock> sessptr;
}

#endif // AUGAS_SESS_HPP
