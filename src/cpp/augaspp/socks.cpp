/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
    AUG_CTXDEBUG2(aug_tlx, "removing sock: id=[%u]", sock.id());
    socks_.erase(sock.id());
}

void
socks::insert(const sockptr& sock)
{
    AUG_CTXDEBUG2(aug_tlx, "adding sock: id=[%u]", sock->id());
    socks_.insert(make_pair(sock->id(), sock));
}

void
socks::teardown(chans& chans, const aug_timeval& now)
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
