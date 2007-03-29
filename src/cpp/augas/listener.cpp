/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/listener.hpp"

using namespace aug;
using namespace augas;
using namespace std;

augas_object&
listener::do_object()
{
    return sock_;
}

const augas_object&
listener::do_object() const
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
