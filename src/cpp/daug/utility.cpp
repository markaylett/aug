/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "daug/utility.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augas.h"

namespace {

    void
    stop()
    {
    }
    int
    start(augas_serv* serv)
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
    closed(const augas_object* sock)
    {
    }
    int
    accept(augas_object* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    void
    connected(augas_object* sock, const char* addr, unsigned short port)
    {
    }
    void
    data(const augas_object* sock, const void* buf, size_t len)
    {
    }
    void
    rdexpire(const augas_object* sock, unsigned* ms)
    {
    }
    void
    wrexpire(const augas_object* sock, unsigned* ms)
    {
    }
    void
    expire(const augas_object* timer, unsigned* ms)
    {
    }
    int
    authcert(const augas_object* sock, const char* subject,
             const char* issuer)
    {
        return 0;
    }
}

void
augas::setdefaults(augas_module& dst, const augas_module& src,
                   void (*teardown)(const augas_object*))
{
    dst.stop_ = src.stop_ ? src.stop_ : stop;
    dst.start_ = src.start_ ? src.start_ : start;
    dst.reconf_ = src.reconf_ ? src.reconf_ : reconf;
    dst.event_ = src.event_ ? src.event_ : event;
    dst.closed_ = src.closed_ ? src.closed_ : closed;
    dst.teardown_ = src.teardown_ ? src.teardown_ : teardown;
    dst.accept_ = src.accept_ ? src.accept_ : accept;
    dst.connected_ = src.connected_ ? src.connected_ : connected;
    dst.data_ = src.data_ ? src.data_ : data;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
    dst.authcert_ = src.authcert_ ? src.authcert_ : authcert;
}
