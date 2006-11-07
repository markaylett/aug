/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/utility.hpp"

#include "augas/module.h"

namespace {

    void
    close(const struct augas_session* s)
    {
    }

    int
    open(struct augas_session* s, const char* serv, const char* peer)
    {
        return 0;
    }

    int
    data(const struct augas_session* s, const char* buf, size_t size)
    {
        return 0;
    }

    int
    rdexpire(const struct augas_session* s, unsigned* ms)
    {
        return 0;
    }

    int
    wrexpire(const struct augas_session* s, unsigned* ms)
    {
        return 0;
    }

    int
    stop(const struct augas_session* s)
    {
        return 0;
    }

    int
    event(int type, void* arg)
    {
        return 0;
    }

    int
    expire(void* arg, unsigned id, unsigned* ms)
    {
        return 0;
    }

    int
    reconf(void)
    {
        return 0;
    }
}

void
augas::setdefaults(struct augas_module& dst, const struct augas_module& src)
{
    dst.close_ = src.close_ ? src.close_ : close;
    dst.open_ = src.open_ ? src.open_ : open;
    dst.data_ = src.data_ ? src.data_ : data;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.stop_ = src.stop_ ? src.stop_ : stop;
    dst.event_ = src.event_ ? src.event_ : event;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
    dst.reconf_ = src.reconf_ ? src.reconf_ : reconf;
}
