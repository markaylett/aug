/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECT_HPP
#define AUGRTPP_OBJECT_HPP

#include "augrtpp/serv.hpp"

#include "augsyspp.hpp"

#include "augas.h"

namespace aug {

    class object_base {
    public:
        typedef augas_object ctype;
    private:

        virtual augas_object&
        do_get() = 0;

        virtual const augas_object&
        do_get() const = 0;

        virtual const aug::servptr&
        do_serv() const = 0;

        virtual aug::smartfd
        do_sfd() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        augas_object&
        get()
        {
            return do_get();
        }
        const augas_object&
        get() const
        {
            return do_get();
        }
        const aug::servptr&
        serv() const
        {
            return do_serv();
        }
        aug::smartfd
        sfd() const
        {
            return do_sfd();
        }
        operator augas_object&()
        {
            return do_get();
        }
        operator const augas_object&() const
        {
            return do_get();
        }
    };

    typedef smartptr<object_base> objectptr;

    inline int
    id(const augas_object& ref)
    {
        return static_cast<int>(ref.id_);
    }

    inline void*
    user(const augas_object& ref)
    {
        return ref.user_;
    }
}

#endif // AUGRTPP_OBJECT_HPP
