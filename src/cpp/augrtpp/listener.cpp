/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/listener.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

AUGRTPP_API augas_object&
listener::do_get()
{
    return sock_;
}

AUGRTPP_API const augas_object&
listener::do_get() const
{
    return sock_;
}

AUGRTPP_API const servptr&
listener::do_serv() const
{
    return serv_;
}

AUGRTPP_API smartfd
listener::do_sfd() const
{
    return sfd_;
}

AUGRTPP_API sockstate
listener::do_state() const
{
    return LISTENING;
}

AUGRTPP_API
listener::~listener() AUG_NOTHROW
{
    try {
        serv_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

AUGRTPP_API
listener::listener(const servptr& serv, void* user, const smartfd& sfd)
    : serv_(serv),
      sfd_(sfd)
{
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
