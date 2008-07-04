/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
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

chanptr
listener::do_chan() const
{
    return chan_;
}

sockstate
listener::do_state() const
{
    return LISTENING;
}

listener::~listener() AUG_NOTHROW
{
    try {
        session_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

listener::listener(const sessionptr& session, void* user, const chanptr& chan)
    : session_(session),
      chan_(chan)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
