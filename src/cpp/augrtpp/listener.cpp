/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/listener.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

augas_object&
listener::do_get()
{
    return sock_;
}

const augas_object&
listener::do_get() const
{
    return sock_;
}

const servptr&
listener::do_serv() const
{
    return serv_;
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
        serv_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

listener::listener(const servptr& serv, void* user, const smartfd& sfd)
    : serv_(serv),
      sfd_(sfd)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
