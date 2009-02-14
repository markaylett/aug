/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGASPP_OBJECT_HPP
#define AUGASPP_OBJECT_HPP

#include "augaspp/session.hpp"

#include "augctxpp/mpool.hpp"

#include "augmod.h"

namespace aug {

    class object_base {
    public:
        typedef mod_handle ctype;
    private:

        virtual mod_handle&
        do_get() = 0;

        virtual const mod_handle&
        do_get() const = 0;

        virtual const sessionptr&
        do_session() const = 0;

    public:
        virtual
        ~object_base() AUG_NOTHROW;

        mod_handle&
        get()
        {
            return do_get();
        }
        const mod_handle&
        get() const
        {
            return do_get();
        }
        const sessionptr&
        session() const
        {
            return do_session();
        }
        operator mod_handle&()
        {
            return do_get();
        }
        operator const mod_handle&() const
        {
            return do_get();
        }
    };

    typedef smartptr<object_base> objectptr;

    inline mod_id
    id(const mod_handle& ref)
    {
        return ref.id_;
    }

    inline void*
    user(const mod_handle& ref)
    {
        return ref.user_;
    }
}

#endif // AUGASPP_OBJECT_HPP
