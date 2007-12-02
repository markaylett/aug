/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECT_HPP
#define AUGRTPP_OBJECT_HPP

#include "augaspp/session.hpp"

#include "maud.h"

namespace aug {

    class object_base {
    public:
        typedef maud_handle ctype;
    private:

        virtual maud_handle&
        do_get() = 0;

        virtual const maud_handle&
        do_get() const = 0;

        virtual const sessionptr&
        do_session() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        maud_handle&
        get()
        {
            return do_get();
        }
        const maud_handle&
        get() const
        {
            return do_get();
        }
        const sessionptr&
        session() const
        {
            return do_session();
        }
        operator maud_handle&()
        {
            return do_get();
        }
        operator const maud_handle&() const
        {
            return do_get();
        }
    };

    typedef smartptr<object_base> objectptr;

    inline maud_id
    id(const maud_handle& ref)
    {
        return ref.id_;
    }

    inline void*
    user(const maud_handle& ref)
    {
        return ref.user_;
    }
}

#endif // AUGRTPP_OBJECT_HPP
