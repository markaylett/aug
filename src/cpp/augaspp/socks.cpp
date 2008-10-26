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

void
socks::clear()
{
    socks_.clear();
}

void
socks::erase(mod_id id)
{
    container::iterator it(socks_.find(id));
    if (it != socks_.end()) {
        AUG_CTXDEBUG2(aug_tlx, "removing sock: id=[%u]", id);
        socks_.erase(it);
    }
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
socks::teardown(chans& chans, const timeval& now)
{
    // Ids are stored in reverse order using the greater<> predicate.

    container::const_iterator it(socks_.begin()), end(socks_.end());
    for (; it != end; ++it) {

        AUG_CTXDEBUG2(aug_tlx, "teardown: id=[%u]", it->first);

        try {
            connptr cptr(smartptr_cast<conn_base>(it->second));
            if (null == cptr) {

                // Not a stream.

                chanptr chan(findchan(chans, it->first));
                closechan(chan);

            } else {
                cptr->teardown(now);
            }

        } AUG_PERRINFOCATCH;
    }
}

bool
socks::empty() const
{
    return socks_.empty();
}

bool
socks::exists(mod_id id) const
{
    return socks_.find(id) != socks_.end();
}

sockptr
socks::get(mod_id id) const
{
    container::const_iterator it(socks_.find(id));
    if (it == socks_.end())
        throw aug_error(__FILE__, __LINE__, AUG_ESTATE,
                        AUG_MSG("sock not found: id=[%u]"), id);
    return it->second;
}
