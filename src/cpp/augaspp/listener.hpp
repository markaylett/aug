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
#ifndef AUGASPP_LISTENER_HPP
#define AUGASPP_LISTENER_HPP

#include "augaspp/sock.hpp"

#include "augsyspp.hpp"

namespace aug {

    class listener : public sock_base, public mpool_ops {

        sessionptr session_;
        mod_handle sock_;
        sockstate state_;

        // object_base.

        mod_handle&
        do_get();

        const mod_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        // sock_base.

        void
        do_error(const char* desc);

        void
        do_shutdown(chanref chan, unsigned flags, const aug_timeval& now);

        sockstate
        do_state() const;

    public:
        ~listener() AUG_NOTHROW;

        listener(const sessionptr& session, unsigned id, objectref ob);
    };

    typedef smartptr<listener> listenerptr;
}

#endif // AUGASPP_LISTENER_HPP
