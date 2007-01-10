
/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD
#include "augas/listener.hpp"

using namespace aug;
using namespace augas;
using namespace std;

augas_file&
listener::do_file()
{
    return file_;
}

int
listener::do_fd() const
{
    return sfd_.get();
}

const augas_file&
listener::do_file() const
{
    return file_;
}

const sessptr&
listener::do_sess() const
{
    return sess_;
}

listener::~listener() AUG_NOTHROW
{
    try {
        sess_->close(file_);
    } AUG_PERRINFOCATCH;
}

listener::listener(const sessptr& sess, const smartfd& sfd, augas_id id,
                   void* user)
    : sess_(sess),
      sfd_(sfd)
{
    file_.sess_ = cptr(*sess_);
    file_.id_ = id;
    file_.user_ = user;
}
