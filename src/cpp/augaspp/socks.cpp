/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
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
    idtosd_.clear();
    socks_.clear();
}

void
socks::erase(const sock_base& sock)
{
    AUG_CTXDEBUG2(aug_tlx, "removing sock: id=[%d], fd=[%d]", id(sock),
                  sock.sd().get());

    idtosd_.erase(id(sock));
    socks_.erase(sock.sd());
}

void
socks::insert(const sockptr& sock)
{
    AUG_CTXDEBUG2(aug_tlx, "adding sock: id=[%d], fd=[%d]", id(*sock),
                  sock->sd().get());

    socks_.insert(make_pair(sock->sd(), sock));
    idtosd_.insert(make_pair(id(*sock), sock->sd()));
}

void
socks::update(const sockptr& sock, sdref prev)
{
    AUG_CTXDEBUG2(aug_tlx, "updating sock: id=[%d], fd=[%d], prev=[%d]",
                  id(*sock), sock->sd().get(), prev.get());

    socks_.insert(make_pair(sock->sd(), sock));
    socks_.erase(prev);

    pair<idtosd::iterator, bool> xy
        (idtosd_.insert(make_pair(id(*sock), sock->sd())));
    if (!xy.second)
        xy.first->second = sock->sd();
}

void
socks::teardown(const timeval& now)
{
    // Ids are stored in reverse order using the the greater<> predicate.

    idtosd::iterator rit(idtosd_.begin()), rend(idtosd_.end());
    while (rit != rend) {

        AUG_CTXDEBUG2(aug_tlx, "teardown: id=[%d], fd=[%d]", rit->first,
                      rit->second.get());

        map<sdref, sockptr>::iterator it(socks_.find(rit->second));
        if (it == socks_.end())
            throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                            AUG_MSG("sock not found: fd=[%d]"),
                            rit->second.get());

        connptr cptr(smartptr_cast<conn_base>(it->second));
        if (null != cptr) {
            ++rit;
            try {
                cptr->teardown(now);
            } AUG_PERRINFOCATCH;
            continue;
        }

        // Erase listener.

        idtosd_.erase(rit++);
        socks_.erase(it);
    }
}

sockptr
socks::getbysd(sdref sd) const
{
    map<sdref, sockptr>::const_iterator it(socks_.find(sd));
    if (it == socks_.end())
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        AUG_MSG("sock not found: fd=[%d]"), sd.get());
    return it->second;
}

sockptr
socks::getbyid(mod_id id) const
{
    idtosd::const_iterator it(idtosd_.find(id));
    if (it == idtosd_.end())
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        AUG_MSG("sock not found: id=[%d]"), id);
    return getbysd(it->second);
}

bool
socks::empty() const
{
    return socks_.empty();
}
