/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/listener.hpp"

using namespace aug;
using namespace augas;
using namespace std;

augas_sock&
listener::do_sock()
{
    return sock_;
}

const augas_sock&
listener::do_sock() const
{
    return sock_;
}

const sessptr&
listener::do_sess() const
{
    return sess_;
}

smartfd
listener::do_sfd() const
{
    return sfd_;
}

listener::~listener() AUG_NOTHROW
{
    try {
        sess_->closed(sock_);
    } AUG_PERRINFOCATCH;
}

listener::listener(const sessptr& sess, void* user, const smartfd& sfd)
    : sess_(sess),
      sfd_(sfd)
{
    sock_.sess_ = cptr(*sess);
    sock_.id_ = aug_nextid();
    sock_.user_ = user;
}
