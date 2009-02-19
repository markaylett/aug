/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define AUGD_BUILD
#include "augd/utility.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augabi.h"
#include "augmod.h"

namespace {

    void
    stop()
    {
    }
    mod_bool
    start(mod_session* session)
    {
        return MOD_TRUE;
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
    mod_bool
    accepted(mod_handle* sock, const char* name)
    {
        return MOD_TRUE;
    }
    void
    connected(mod_handle* sock, const char* name)
    {
    }
    mod_bool
    auth(const mod_handle* sock, const char* subject, const char* issuer)
    {
        return MOD_TRUE;
    }
    void
    recv(const mod_handle* sock, const void* buf, size_t len)
    {
    }
    void
    error(const mod_handle* sock, const char* desc)
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
}

void
augd::setdefaults(mod_module& dst, const mod_module& src,
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
    dst.auth_ = src.auth_ ? src.auth_ : auth;
    dst.recv_ = src.recv_ ? src.recv_ : recv;
    dst.error_ = src.error_ ? src.error_ : error;
    dst.rdexpire_ = src.rdexpire_ ? src.rdexpire_ : rdexpire;
    dst.wrexpire_ = src.wrexpire_ ? src.wrexpire_ : wrexpire;
    dst.expire_ = src.expire_ ? src.expire_ : expire;
}
