/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/utility.hpp"

#include "augas.h"

namespace {

    void
    destroy(const augas_serv* serv)
    {
    }
    int
    create(augas_serv* serv)
    {
        return 0;
    }
    int
    reconf(const augas_serv* serv)
    {
        return 0;
    }
    int
    event(const augas_serv* serv, const char* from, const augas_event* event)
    {
        return 0;
    }
    void
    closed(const augas_object* sock)
    {
    }
    int
    teardown(const augas_object* sock)
    {
        return 0;
    }
    int
    accept(augas_object* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    connected(augas_object* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    data(const augas_object* sock, const char* buf, size_t size)
    {
        return 0;
    }
    int
    rdexpire(const augas_object* sock, unsigned* ms)
    {
        return 0;
    }
    int
    wrexpire(const augas_object* sock, unsigned* ms)
    {
        return 0;
    }
    int
    expire(const augas_object* timer, unsigned* ms)
    {
        return 0;
    }
}

void
augas::setdefaults(augas_module& dst, const augas_module& src)
{
    dst.destroy_ = src.destroy_ ? src.destroy_ : destroy;
    dst.create_ = src.create_ ? src.create_ : create;
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
}
