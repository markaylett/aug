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
#ifndef AUGASPP_SOCKS_HPP
#define AUGASPP_SOCKS_HPP

#include "augaspp/sock.hpp"

#include <map>

struct timeval;

namespace aug {

    class chans;

    class socks : public mpool_ops {

        typedef std::map<mod_id, sockptr, std::greater<mod_id> > container;

        container socks_;

        socks(const socks& rhs);

        socks&
        operator =(const socks& rhs);

    public:
        ~socks() AUG_NOTHROW;

        socks()
        {
        }

        void
        clear();

        void
        erase(mod_id id);

        void
        erase(const sock_base& sock);

        void
        insert(const sockptr& sock);

        void
        teardown(chans& chans, const timeval& tv);

        bool
        empty() const;

        bool
        exists(mod_id id) const;

        // Throws if id does not exist.

        sockptr
        get(mod_id id) const;
    };

    class scoped_insert {

        socks& socks_;
        sockptr sock_;

        scoped_insert(const scoped_insert& rhs);

        scoped_insert&
        operator =(const scoped_insert& rhs);

    public:
        ~scoped_insert() AUG_NOTHROW
        {
            if (null != sock_)
                socks_.erase(*sock_);
        }
        scoped_insert(socks& socks, const sockptr& sock)
            : socks_(socks),
              sock_(sock)
        {
            socks.insert(sock);
        }
        void
        commit()
        {
            sock_ = null;
        }
    };
}

#endif // AUGASPP_SOCKS_HPP
