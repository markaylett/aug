/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECT_HPP
#define AUGRTPP_OBJECT_HPP

#include "augrtpp/serv.hpp"

#include "augas.h"

namespace aug {

    class AUGRTPP_API object_base {
    public:
        typedef augas_object ctype;
    private:

        virtual augas_object&
        do_get() = 0;

        virtual const augas_object&
        do_get() const = 0;

        virtual const servptr&
        do_serv() const = 0;

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
        const servptr&
        serv() const
        {
            return do_serv();
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

    inline augas_id
    id(const augas_object& ref)
    {
        return ref.id_;
    }

    inline void*
    user(const augas_object& ref)
    {
        return ref.user_;
    }
}

#endif // AUGRTPP_OBJECT_HPP
