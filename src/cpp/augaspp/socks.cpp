/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGASPP_BUILD
#include "augaspp/socks.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augaspp/conn.hpp"

using namespace aug;
using namespace std;

socks::~socks() AUG_NOTHROW
{
}

bool
socks::send(mod_id cid, const void* buf, size_t len, const timeval& now)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (cptr == null || !sendable(*cptr))
        return false;

    cptr->send(buf, len, now);
    return true;
}

bool
socks::sendv(mod_id cid, blobref blob, const timeval& now)
{
    connptr cptr(smartptr_cast<conn_base>(getbyid(cid)));
    if (cptr == null || !sendable(*cptr))
        return false;

    cptr->sendv(blob, now);
    return true;
}

void
socks::clear()
{
    socks_.clear();
}

void
socks::erase(const sock_base& sock)
{
    AUG_CTXDEBUG2(aug_tlx, "removing sock: id=[%u]", id(sock));
    socks_.erase(id(sock));
}

void
socks::insert(const sockptr& sock)
{
    AUG_CTXDEBUG2(aug_tlx, "adding sock: id=[%u]", id(*sock));
    socks_.insert(make_pair(id(*sock), sock));
}

void
socks::teardown(const timeval& now)
{
    // Ids are stored in reverse order using the the greater<> predicate.

    container::iterator it(socks_.begin()), end(socks_.end());
    while (it != end) {

        AUG_CTXDEBUG2(aug_tlx, "teardown: id=[%u]", it->first);

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null != cptr) {
            ++it;
            try {
                cptr->teardown(now);
            } AUG_PERRINFOCATCH;
            continue;
        }

        // Erase listener.

        socks_.erase(it++);
    }
}

sockptr
socks::getbyid(mod_id id) const
{
    container::const_iterator it(socks_.find(id));
    if (it == socks_.end())
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        AUG_MSG("sock not found: id=[%u]"), id);
    return it->second;
}

bool
socks::empty() const
{
    return socks_.empty();
}
