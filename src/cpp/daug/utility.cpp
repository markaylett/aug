/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define DAUG_BUILD
#include "daug/utility.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augabi.h"
#include "augmod.h"

namespace {

    void
    stop()
    {
    }
    int
    start(mod_session* session)
    {
        return 0;
    }
    void
    reconf()
    {
    }
    void
    event(const char* from, const char* type, aug_object* ob)
    {
    }
    void
    closed(const mod_handle* sock)
    {
    }
    int
    accepted(mod_handle* sock, const char* addr, unsigned short port)
    {
        return 0;
    }
    void
    connected(mod_handle* sock, const char* addr, unsigned short port)
    {
    }
    void
    data(const mod_handle* sock, const void* buf, size_t len)
    {
    }
    void
    rdexpire(const mod_handle* sock, unsigned* ms)
    {
    }
    void
    wrexpire(const mod_handle* sock, unsigned* ms)
    {
    }
    void
    expire(const mod_handle* timer, unsigned* ms)
    {
    }
    int
    authcert(const mod_handle* sock, const char* subject,
             const char* issuer)
    {
        return 0;
    }
}

void
daug::setdefaults(mod_module& dst, const mod_module& src,
                  void (*teardown)(const mod_handle*))
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
