/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/listener.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

augrt_object&
listener::do_get()
{
    return sock_;
}

const augrt_object&
listener::do_get() const
{
    return sock_;
}

const sessionptr&
listener::do_session() const
{
    return session_;
}

smartfd
listener::do_sfd() const
{
    return sfd_;
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

listener::listener(const sessionptr& session, void* user, const smartfd& sfd)
    : session_(session),
      sfd_(sfd)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
