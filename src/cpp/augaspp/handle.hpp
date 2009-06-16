/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGASPP_HANDLE_HPP
#define AUGASPP_HANDLE_HPP

#include "augaspp/session.hpp"

#include "augctxpp/mpool.hpp"

#include "augabipp.hpp"

#include "augmod.h"

namespace aug {

    class handle_base {
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
        ~handle_base() AUG_NOTHROW;

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
        mod_id
        id() const
        {
            return get().id_;
        }
        objectptr
        ob() const
        {
            return object_retain(obptr(get().ob_));
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

    typedef smartptr<handle_base> handleptr;

    class handle {

        mod_handle handle_;

        handle(const handle& rhs);

        handle&
        operator =(const handle& rhs);

    public:
        ~handle() AUG_NOTHROW
        {
            aug_assign(handle_.ob_, 0);
        }
        handle(aug_id id, objectref ob)
        {
            handle_.id_ = id;
            handle_.ob_ = ob.get();
            if (handle_.ob_)
                aug_retain(handle_.ob_);
        }
        mod_handle&
        get()
        {
            return handle_;
        }
        const mod_handle&
        get() const
        {
            return handle_;
        }
        mod_id
        id() const
        {
            return handle_.id_;
        }
        objectptr
        ob() const
        {
            return object_retain(obptr(handle_.ob_));
        }
        operator mod_handle&()
        {
            return get();
        }
        operator const mod_handle&() const
        {
            return get();
        }
    };
}

#endif // AUGASPP_HANDLE_HPP
