/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_INET_HPP
#define AUGNETPP_INET_HPP

#include "augnetpp/config.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/socket.hpp" // close()
#include "augsyspp/smartfd.hpp"

#include "augnet/inet.h"

namespace aug {

    inline autosd
    tcpconnect(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_tcpconnect(host, serv, &ep), close);
        if (null == sd)
            failerror();

        return sd;
    }

    inline autosd
    tcplisten(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_tcplisten(host, serv, &ep), close);
        if (null == sd)
            failerror();

        return sd;
    }

    inline autosd
    udpclient(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_udpclient(host, serv, &ep), close);
        if (null == sd)
            failerror();

        return sd;
    }

    inline autosd
    udpconnect(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_udpconnect(host, serv, &ep), close);
        if (null == sd)
            failerror();

        return sd;
    }

    inline autosd
    udpserver(const char* host, const char* serv, aug_endpoint& ep)
    {
        autosd sd(aug_udpserver(host, serv, &ep), close);
        if (null == sd)
            failerror();

        return sd;
    }

    inline aug_hostserv&
    parsehostserv(const char* src, aug_hostserv& dst)
    {
        return *verify(aug_parsehostserv(src, &dst));
    }

    inline void
    setnodelay(sdref ref, bool on)
    {
        int value(on ? 1 : 0);
        verify(aug_setnodelay(ref.get(), value));
    }

    inline bool
    established(sdref ref)
    {
        return AUG_FAILNONE == verify(aug_established(ref.get()))
            ? false : true;
    }
}

#endif // AUGNETPP_INET_HPP
