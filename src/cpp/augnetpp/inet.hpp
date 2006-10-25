/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_INET_HPP
#define AUGNETPP_INET_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/inet.h"

namespace aug {

    inline smartfd
    tcpconnect(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_tcpconnect(host, serv, &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline smartfd
    tcplisten(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_tcplisten(host, serv, &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline smartfd
    udpclient(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpclient(host, serv, &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline smartfd
    udpconnect(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpconnect(host, serv, &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline smartfd
    udpserver(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpserver(host, serv, &ep)));
        if (null == sfd)
            fail();

        return sfd;
    }

    inline void
    setnodelay(fdref ref, bool on)
    {
        int value(on ? 1 : 0);
        verify(aug_setnodelay(ref.get(), value));
    }

    inline struct aug_hostserv&
    parsehostserv(const char* src, struct aug_hostserv& dst)
    {
        return *verify(aug_parsehostserv(src, &dst));
    }
}

#endif // AUGNETPP_INET_HPP
