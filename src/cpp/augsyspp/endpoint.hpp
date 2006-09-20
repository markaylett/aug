/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_ENDPOINT_HPP
#define AUGSYSPP_ENDPOINT_HPP

#include "augsyspp/socket.hpp"

namespace aug {

    inline aug_endpoint&
    clear(struct aug_endpoint& ep)
    {
        bzero(&ep, sizeof(struct aug_endpoint));
        return ep;
    }

    inline aug_endpoint&
    setfamily(struct aug_endpoint& ep, short family)
    {
        switch (ep.un_.family_ = family) {
        case AF_INET:
            ep.len_ = sizeof(struct sockaddr_in);
            break;
        case AF_INET6:
            ep.len_ = sizeof(struct sockaddr_in6);
            break;
        default:
            ep.len_ = AUG_MAXADDRLEN;
        }
        return ep;
    }

    inline aug_endpoint&
    setport(struct aug_endpoint& ep, unsigned short port)
    {
        ep.un_.all_.port_ = port;
        return ep;
    }

    inline aug_endpoint&
    setsockaddr(struct aug_endpoint& ep, const struct sockaddr_in& ipv4)
    {
        ep.len_ = sizeof(struct sockaddr_in);
        memcpy(&ep.un_.ipv4_, &ipv4, sizeof(ep.un_.ipv4_));
        return ep;
    }

#if !defined(AUG_NOIPV6)
    inline aug_endpoint&
    setsockaddr(struct aug_endpoint& ep, const struct sockaddr_in6& ipv6)
    {
        ep.len_ = sizeof(struct sockaddr_in6);
        memcpy(&ep.un_.ipv6_, &ipv6, sizeof(ep.un_.ipv6_));
        return ep;
    }
#endif // !AUG_NOIPV6

    inline short
    family(const struct aug_endpoint& ep)
    {
        return ep.un_.family_;
    }

    inline socklen_t
    len(const struct aug_endpoint& ep)
    {
        return ep.len_;
    }

    class endpoint {
    public:
        typedef struct aug_endpoint ctype;
    private:
        struct aug_endpoint ep_;

    public:
        endpoint(const null_&) NOTHROW
        {
            clear(ep_);
        }

        explicit
        endpoint(const struct aug_inetaddr& addr, unsigned short port = 0)
        {
            setport(ep_, port);
            setinetaddr(ep_, addr);
        }

        endpoint&
        operator =(const null_&) NOTHROW
        {
            clear(ep_);
            return *this;
        }

        operator struct aug_endpoint&()
        {
            return ep_;
        }

        operator const struct aug_endpoint&() const
        {
            return ep_;
        }
    };
}

inline bool
isnull(const struct aug_endpoint& ep)
{
    return 0 == aug::family(ep);
}

inline bool
isnull(const aug::endpoint& ep)
{
    return 0 == aug::family(ep);
}

#endif // AUGSYSPP_ENDPOINT_HPP
