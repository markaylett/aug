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
#ifndef AUGSYSPP_INETADDR_HPP
#define AUGSYSPP_INETADDR_HPP

#include "augsyspp/socket.hpp"

#include "augctxpp/mpool.hpp"

namespace aug {

    inline aug_inetaddr&
    clear(aug_inetaddr& addr)
    {
        memset(&addr, 0, sizeof(aug_inetaddr));
        return addr;
    }

    inline aug_inetaddr&
    setfamily(aug_inetaddr& addr, short family)
    {
        addr.family_ = family;
        return addr;
    }

    inline aug_inetaddr&
    setinaddr(aug_inetaddr& addr, const in_addr& ipv4)
    {
        addr.family_ = AF_INET;
        addr.un_.ipv4_.s_addr = ipv4.s_addr;
        return addr;
    }

#if HAVE_IPV6
    inline aug_inetaddr&
    setinaddr(aug_inetaddr& addr, const in6_addr& ipv6)
    {
        addr.family_ = AF_INET6;
        memcpy(&addr.un_.ipv6_, &ipv6, sizeof(addr.un_.ipv6_));
        return addr;
    }
#endif // HAVE_IPV6

    inline short
    family(const aug_inetaddr& addr)
    {
        return addr.family_;
    }

    class inetaddr : public mpool_ops {
    public:
        typedef aug_inetaddr ctype;
    private:
        aug_inetaddr addr_;

    public:
        inetaddr(const null_&) AUG_NOTHROW
        {
            clear(addr_);
        }

        inetaddr(int af, const char* src)
        {
            inetpton(af, src, addr_);
        }

        explicit
        inetaddr(const char* src)
        {
            inetpton(src, addr_);
        }

        inetaddr&
        operator =(const null_&) AUG_NOTHROW
        {
            clear(addr_);
            return *this;
        }

        operator aug_inetaddr&()
        {
            return addr_;
        }

        operator const aug_inetaddr&() const
        {
            return addr_;
        }
    };
}

inline bool
isnull(const aug_inetaddr& addr)
{
    return 0 == aug::family(addr);
}

inline bool
isnull(const aug::inetaddr& addr)
{
    return 0 == aug::family(addr);
}

#endif // AUGSYSPP_INETADDR_HPP
