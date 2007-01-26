/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_SOCK_HPP
#define AUGAS_SOCK_HPP

#include "augas.h"
#include "augas/sess.hpp"

#include "augsyspp.hpp"

namespace augas {

    class sock_base {
    public:
        typedef augas_sock ctype;
    private:

        virtual augas_sock&
        do_sock() = 0;

        virtual const augas_sock&
        do_sock() const = 0;

        virtual const sessptr&
        do_sess() const = 0;

        virtual aug::smartfd
        do_sfd() const = 0;

    public:
        virtual
        ~sock_base() AUG_NOTHROW;

        augas_sock&
        sock()
        {
            return do_sock();
        }
        const augas_sock&
        sock() const
        {
            return do_sock();
        }
        const sessptr&
        sess() const
        {
            return do_sess();
        }
        aug::smartfd
        sfd() const
        {
            return do_sfd();
        }
        augas_id
        id() const
        {
            return do_sock().id_;
        }
        void*
        user() const
        {
            return do_sock().user_;
        }
        operator augas_sock&()
        {
            return do_sock();
        }
        operator const augas_sock&() const
        {
            return do_sock();
        }
    };

    typedef aug::smartptr<sock_base> sockptr;
}

#endif // AUGAS_SOCK_HPP
