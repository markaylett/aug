/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augrtpp/socks.hpp"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augrtpp/conn.hpp"

using namespace aug;
using namespace std;

bool
socks::append(augas_id cid, const aug_var& var)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (!sendable(*cptr))
        return false;

    cptr->append(var);
    return true;
}

bool
socks::append(augas_id cid, const void* buf, size_t len)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (!sendable(*cptr))
        return false;

    cptr->append(buf, len);
    return true;
}

void
socks::clear()
{
    idtofd_.clear();
    socks_.clear();
}

void
socks::erase(const sock_base& sock)
{
    AUG_DEBUG2("removing sock: id=[%d], fd=[%d]", id(sock), sock.sfd().get());

    idtofd_.erase(id(sock));
    socks_.erase(sock.sfd().get());
}

void
socks::insert(const sockptr& sock)
{
    AUG_DEBUG2("adding sock: id=[%d], fd=[%d]", id(*sock), sock->sfd().get());

    socks_.insert(make_pair(sock->sfd().get(), sock));
    idtofd_.insert(make_pair(id(*sock), sock->sfd().get()));
}

void
socks::update(const sockptr& sock, fdref prev)
{
    AUG_DEBUG2("updating sock: id=[%d], fd=[%d], prev=[%d]", id(*sock),
               sock->sfd().get(), prev.get());

    socks_.insert(make_pair(sock->sfd().get(), sock));
    socks_.erase(prev.get());

    idtofd_[id(*sock)] = sock->sfd().get();
}

void
socks::teardown()
{
    // Ids are stored in reverse order using the the greater<> predicate.

    idtofd::iterator rit(idtofd_.begin()), rend(idtofd_.end());
    while (rit != rend) {

        AUG_DEBUG2("teardown: id=[%d], fd=[%d]", rit->first, rit->second);

        map<int, sockptr>::iterator it(socks_.find(rit->second));
        if (it == socks_.end())
            throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                              AUG_MSG("sock not found: fd=[%d]"),
                              rit->second);

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null != cptr) {
            ++rit;
            try {
                cptr->teardown();
            } AUG_PERRINFOCATCH;
            continue;
        }

        // Not a connection.

        idtofd_.erase(rit++);
        socks_.erase(it);
    }
}

sockptr
socks::getbyfd(fdref fd) const
{
    map<int, sockptr>::const_iterator it(socks_.find(fd.get()));
    if (it == socks_.end())
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          AUG_MSG("sock not found: fd=[%d]"), fd.get());
    return it->second;
}

sockptr
socks::getbyid(augas_id id) const
{
    idtofd::const_iterator it(idtofd_.find(id));
    if (it == idtofd_.end())
        throw local_error(__FILE__, __LINE__, AUG_ESTATE,
                          AUG_MSG("sock not found: id=[%d]"), id);
    return getbyfd(it->second);
}

bool
socks::empty() const
{
    return socks_.empty();
}
