/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_OBJECT_HPP
#define AUGAS_OBJECT_HPP

#include "augas.h"
#include "daug/serv.hpp"

#include "augsyspp.hpp"

namespace augas {

    class object_base {
    public:
        typedef augas_object ctype;
    private:

        virtual augas_object&
        do_object() = 0;

        virtual const augas_object&
        do_object() const = 0;

        virtual const servptr&
        do_serv() const = 0;

        virtual aug::smartfd
        do_sfd() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        augas_object&
        object()
        {
            return do_object();
        }
        const augas_object&
        object() const
        {
            return do_object();
        }
        const servptr&
        serv() const
        {
            return do_serv();
        }
        aug::smartfd
        sfd() const
        {
            return do_sfd();
        }
        augas_id
        id() const
        {
            return do_object().id_;
        }
        void*
        user() const
        {
            return do_object().user_;
        }
        operator augas_object&()
        {
            return do_object();
        }
        operator const augas_object&() const
        {
            return do_object();
        }
    };

    typedef aug::smartptr<object_base> objectptr;
}

#endif // AUGAS_OBJECT_HPP
