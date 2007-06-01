/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECT_HPP
#define AUGRTPP_OBJECT_HPP

#include "augrtpp/session.hpp"

#include "augrt.h"

namespace aug {

    class object_base {
    public:
        typedef augrt_object ctype;
    private:

        virtual augrt_object&
        do_get() = 0;

        virtual const augrt_object&
        do_get() const = 0;

        virtual const sessionptr&
        do_session() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        augrt_object&
        get()
        {
            return do_get();
        }
        const augrt_object&
        get() const
        {
            return do_get();
        }
        const sessionptr&
        session() const
        {
            return do_session();
        }
        operator augrt_object&()
        {
            return do_get();
        }
        operator const augrt_object&() const
        {
            return do_get();
        }
    };

    typedef smartptr<object_base> objectptr;

    inline augrt_id
    id(const augrt_object& ref)
    {
        return ref.id_;
    }

    inline void*
    user(const augrt_object& ref)
    {
        return ref.user_;
    }
}

#endif // AUGRTPP_OBJECT_HPP
