/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_INETADDR_HPP
#define AUGSYSPP_INETADDR_HPP

#include "augsyspp/socket.hpp"

namespace aug {

    inline aug_inetaddr&
    clear(struct aug_inetaddr& addr)
    {
        bzero(&addr, sizeof(struct aug_inetaddr));
        return addr;
    }

    inline aug_inetaddr&
    setfamily(struct aug_inetaddr& addr, short family)
    {
        addr.family_ = family;
        return addr;
    }

    inline aug_inetaddr&
    setinaddr(struct aug_inetaddr& addr, const struct in_addr& ipv4)
    {
        addr.family_ = AF_INET;
        addr.un_.ipv4_.s_addr = ipv4.s_addr;
        return addr;
    }

#if !defined(AUG_NOIPV6)
    inline aug_inetaddr&
    setinaddr(struct aug_inetaddr& addr, const struct in6_addr& ipv6)
    {
        addr.family_ = AF_INET6;
        memcpy(&addr.un_.ipv6_, &ipv6, sizeof(addr.un_.ipv6_));
        return addr;
    }
#endif // !AUG_NOIPV6

    inline short
    family(const struct aug_inetaddr& addr)
    {
        return addr.family_;
    }

    class inetaddr {
    public:
        typedef struct aug_inetaddr ctype;
    private:
        struct aug_inetaddr addr_;

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

        operator struct aug_inetaddr&()
        {
            return addr_;
        }

        operator const struct aug_inetaddr&() const
        {
            return addr_;
        }
    };
}

inline bool
isnull(const struct aug_inetaddr& addr)
{
    return 0 == aug::family(addr);
}

inline bool
isnull(const aug::inetaddr& addr)
{
    return 0 == aug::family(addr);
}

#endif // AUGSYSPP_INETADDR_HPP
