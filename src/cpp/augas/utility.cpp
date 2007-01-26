/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/utility.hpp"

#include "augas.h"

namespace {

    void
    term(const augas_sess* sess)
    {
    }
    int
    init(augas_sess* sess)
    {
        return 0;
    }
    int
    event(const augas_sess* sess, int type, void* user)
    {
        return 0;
    }
    int
    expire(const augas_sess* sess, int tid, void* user, unsigned* ms)
    {
        return 0;
    }
    int
    reconf(const augas_sess* sess)
    {
        return 0;
    }
    void
    closed(const augas_sock* sock)
    {
    }
    int
    accept(augas_sock* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    connected(augas_sock* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    data(const augas_sock* sock, const char* buf, size_t size)
    {
        return 0;
    }
    int
    rdexpire(const augas_sock* sock, unsigned* ms)
    {
        return 0;
    }
    int
    wrexpire(const augas_sock* sock, unsigned* ms)
    {
        return 0;
    }
    int
    teardown(const augas_sock* sock)
    {
        return 0;
    }
}

void
augas::setdefaults(augas_module& dst, const augas_module& src)
{
    dst.term_ = src.term_ ? src.term_ : term;
    dst.init_ = src.init_ ? src.init_ : init;
    dst.event_ = src.event_ ? src.event_ : event;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
    dst.reconf_ = src.reconf_ ? src.reconf_ : reconf;
    dst.closed_ = src.closed_ ? src.closed_ : closed;
    dst.accept_ = src.accept_ ? src.accept_ : accept;
    dst.connected_ = src.connected_ ? src.connected_ : connected;
    dst.data_ = src.data_ ? src.data_ : data;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.teardown_ = src.teardown_ ? src.teardown_ : teardown;
}
