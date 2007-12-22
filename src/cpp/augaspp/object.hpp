/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_OBJECT_HPP
#define AUGRTPP_OBJECT_HPP

#include "augaspp/session.hpp"

#include "aum.h"

namespace aug {

    class object_base {
    public:
        typedef aum_handle ctype;
    private:

        virtual aum_handle&
        do_get() = 0;

        virtual const aum_handle&
        do_get() const = 0;

        virtual const sessionptr&
        do_session() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        aum_handle&
        get()
        {
            return do_get();
        }
        const aum_handle&
        get() const
        {
            return do_get();
        }
        const sessionptr&
        session() const
        {
            return do_session();
        }
        operator aum_handle&()
        {
            return do_get();
        }
        operator const aum_handle&() const
        {
            return do_get();
        }
    };

    typedef smartptr<object_base> objectptr;

    inline aum_id
    id(const aum_handle& ref)
    {
        return ref.id_;
    }

    inline void*
    user(const aum_handle& ref)
    {
        return ref.user_;
    }
}

#endif // AUGRTPP_OBJECT_HPP
