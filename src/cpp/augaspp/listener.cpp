/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
listener::do_shutdown(chanref chan, unsigned flags, const timeval& now)
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
}

listener::listener(const sessionptr& session, void* user, unsigned id)
    : session_(session),
      state_(LISTENING)
{
    sock_.id_ = id;
    sock_.user_ = user;
}
