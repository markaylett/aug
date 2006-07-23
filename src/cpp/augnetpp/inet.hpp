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
            throwerrinfo("aug_tcpconnect() failed");

        return sfd;
    }

    inline smartfd
    tcplisten(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_tcplisten(host, serv, &ep)));
        if (null == sfd)
            throwerrinfo("aug_tcplisten() failed");

        return sfd;
    }

    inline smartfd
    udpclient(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpclient(host, serv, &ep)));
        if (null == sfd)
            throwerrinfo("aug_udpclient() failed");

        return sfd;
    }

    inline smartfd
    udpconnect(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpconnect(host, serv, &ep)));
        if (null == sfd)
            throwerrinfo("aug_udpconnect() failed");

        return sfd;
    }

    inline smartfd
    udpserver(const char* host, const char* serv, struct aug_endpoint& ep)
    {
        smartfd sfd(smartfd::attach(aug_udpserver(host, serv, &ep)));
        if (null == sfd)
            throwerrinfo("aug_udpserver() failed");

        return sfd;
    }

    inline void
    setnodelay(fdref ref, bool on)
    {
        int value(on ? 1 : 0);
        if (-1 == aug_setnodelay(ref.get(), value))
            throwerrinfo("aug_setnodelay() failed");
    }

    inline struct aug_hostserv&
    parsehostserv(struct aug_hostserv& dst, const char* src)
    {
        if (!aug_parsehostserv(&dst, src))
            throwerrinfo("aug_parsehostserv() failed");
        return dst;
    }
}

#endif // AUGNETPP_INET_HPP
