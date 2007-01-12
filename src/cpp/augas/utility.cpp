/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/utility.hpp"

#include "augas.h"

namespace {

    void
    closesess(const struct augas_sess* sess)
    {
    }
    int
    opensess(struct augas_sess* sess)
    {
        return 0;
    }
    int
    event(const struct augas_sess* sess, int type, void* user)
    {
        return 0;
    }
    int
    expire(const struct augas_sess* sess, int tid, void* user, unsigned* ms)
    {
        return 0;
    }
    int
    reconf(const struct augas_sess* sess)
    {
        return 0;
    }
    void
    close(const struct augas_file* file)
    {
    }
    int
    accept(struct augas_file* file, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    connect(struct augas_file* file, const char* addr, unsigned short port)
    {
        return 0;
    }
    int
    data(const struct augas_file* file, const char* buf, size_t size)
    {
        return 0;
    }
    int
    rdexpire(const struct augas_file* file, unsigned* ms)
    {
        return 0;
    }
    int
    wrexpire(const struct augas_file* file, unsigned* ms)
    {
        return 0;
    }
    int
    teardown(const struct augas_file* file)
    {
        return 0;
    }
}

void
augas::setdefaults(struct augas_module& dst, const struct augas_module& src)
{
    dst.closesess_ = src.closesess_ ? src.closesess_ : closesess;
    dst.opensess_ = src.opensess_ ? src.opensess_ : opensess;
    dst.event_ = src.event_ ? src.event_ : event;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
    dst.reconf_ = src.reconf_ ? src.reconf_ : reconf;
    dst.close_ = src.close_ ? src.close_ : close;
    dst.accept_ = src.accept_ ? src.accept_ : accept;
    dst.connect_ = src.connect_ ? src.connect_ : connect;
    dst.data_ = src.data_ ? src.data_ : data;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.teardown_ = src.teardown_ ? src.teardown_ : teardown;
}
