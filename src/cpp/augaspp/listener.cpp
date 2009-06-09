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
#define AUGASPP_BUILD
#include "augaspp/listener.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

mod_handle&
listener::do_get()
{
    return sock_;
}

const mod_handle&
listener::do_get() const
{
    return sock_;
}

const sessionptr&
listener::do_session() const
{
    return session_;
}

void
listener::do_error(const char* desc)
{
    return session_->error(sock_, desc);
}

void
listener::do_shutdown(chanref chan, unsigned flags, const aug_timeval& now)
{
    if (SHUTDOWN <= state_)
        return; // Already shutdown.

    aug_ctxinfo(aug_tlx, "shutting listener: id=[%u], flags=[%u]",
                sock_.id_, flags);

    state_ = CLOSED;
    closechan(chan);
}

sockstate
listener::do_state() const
{
    return state_;
}

listener::~listener() AUG_NOTHROW
{
    try {
        session_->closed(sock_);
    } AUG_PERRINFOCATCH;
    aug_assign(sock_.ob_, 0);
}

listener::listener(const sessionptr& session, unsigned id, objectref ob)
    : session_(session),
      state_(LISTENING)
{
    sock_.id_ = id;
    aug_assign(sock_.ob_, ob.get());
}
