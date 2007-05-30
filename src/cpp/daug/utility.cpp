/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/utility.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrt.h"

namespace {

    void
    stop()
    {
    }
    int
    start(augrt_serv* serv)
    {
        return 0;
    }
    void
    reconf()
    {
    }
    void
    event(const char* from, const char* type, const void* user, size_t size)
    {
    }
    void
    closed(const augrt_object* sock)
    {
    }
    int
    accepted(augrt_object* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    void
    connected(augrt_object* sock, const char* addr, unsigned short port)
    {
    }
    void
    data(const augrt_object* sock, const void* buf, size_t len)
    {
    }
    void
    rdexpire(const augrt_object* sock, unsigned* ms)
    {
    }
    void
    wrexpire(const augrt_object* sock, unsigned* ms)
    {
    }
    void
    expire(const augrt_object* timer, unsigned* ms)
    {
    }
    int
    authcert(const augrt_object* sock, const char* subject,
             const char* issuer)
    {
        return 0;
    }
}

void
augrt::setdefaults(augrt_module& dst, const augrt_module& src,
                   void (*teardown)(const augrt_object*))
{
    dst.stop_ = src.stop_ ? src.stop_ : stop;
    dst.start_ = src.start_ ? src.start_ : start;
    dst.reconf_ = src.reconf_ ? src.reconf_ : reconf;
    dst.event_ = src.event_ ? src.event_ : event;
    dst.closed_ = src.closed_ ? src.closed_ : closed;
    dst.teardown_ = src.teardown_ ? src.teardown_ : teardown;
    dst.accepted_ = src.accepted_ ? src.accepted_ : accepted;
    dst.connected_ = src.connected_ ? src.connected_ : connected;
    dst.data_ = src.data_ ? src.data_ : data;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
    dst.authcert_ = src.authcert_ ? src.authcert_ : authcert;
}
