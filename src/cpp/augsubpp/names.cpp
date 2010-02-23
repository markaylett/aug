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
#define AUGSUBPP_BUILD
#include "augsubpp/names.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsubpp/factory.hpp"

using namespace aug;
using namespace std;

namespace {

    void
    clearwild(map<string, outerptr>& nodes, match m)
    {
        map<string, outerptr>::iterator nit(nodes.begin()),
            nend(nodes.end());
        while (nit != nend) {
            if (0 == nit->second->depth())
                nodes.erase(nit++);
            else {
                nit->second->setmatch(m);
                ++nit;
            }
        }
    }
}

// node_base

void
names::do_clear()
{
    nodes_.clear();
}

unsigned
names::do_depth() const
{
    unsigned d = 0;
    nodes::const_iterator nit(nodes_.begin()), nend(nodes_.end());
    for (; nit != nend; ++nit)
        d = std::max(d, 1 + nit->second->depth());
    return d;
}

bool
names::do_ismatch(path_iterator pit, path_iterator pend) const
{
    const match m(do_query(pit, pend));
    return ARCTRANSIT == m || ARCINCLUDE == m;
}

void
names::do_print(const string& path, ostream& os) const
{
    const string prefix(path + '/');
    nodes::const_iterator nit(nodes_.begin()), nend(nodes_.end());
    for (; nit != nend; ++nit)
        nit->second->print(prefix + nit->first, os);
}

// query_base

match
names::do_query(path_iterator pit, path_iterator pend) const
{
    if (pit != pend) {
        nodes::const_iterator nit(nodes_.find(*pit));
        if (nit != nodes_.end())
            return nit->second->query(pit + 1, pend);
    }
    return ARCNOTHING;
}

bool
names::do_isdead() const
{
    nodes::const_iterator nit(nodes_.begin()), nend(nodes_.end());
    for (; nit != nend; ++nit)
        if (!nit->second->isdead())
            return false;
    return true;
}

// inner_base

void
names::do_insertname(const string& name)
{
    nodes::iterator nit(nodes_.find(name));
    if (nit == nodes_.end())
        nit = nodes_.insert(make_pair(name, factory_.createouter()))
            .first;
    nit->second->setmatch(ARCINCLUDE);
}

void
names::do_insertname(const string& name, path_iterator pit,
                      path_iterator pend)
{
    nodes::iterator nit(nodes_.find(name));
    if (nit == nodes_.end())
        nit = nodes_.insert(make_pair(name, factory_.createouter()))
            .first;
    nit->second->insert(*pit, pit + 1, pend);
}

void
names::do_insertrecur()
{
    clear();
}

void
names::do_insertwild()
{
    clearwild(nodes_, ARCINCLUDE);
}

void
names::do_insertwild(path_iterator pit, path_iterator pend)
{
    path_iterator pnext(pit + 1);
    nodes::const_iterator nit(nodes_.begin()), nend(nodes_.end());
    for (; nit != nend; ++nit)
        nit->second->insert(*pit, pnext, pend);
}

void
names::do_erasename(const string& name)
{
    nodes::iterator nit(nodes_.find(name));
    if (nit != nodes_.end()) {
        nit->second->setmatch(ARCEXCLUDE);
        if (nit->second->isdead())
            nodes_.erase(nit);
    }
}

void
names::do_erasename(const string& name, path_iterator pit,
                     path_iterator pend)
{
    nodes::iterator nit(nodes_.find(name));
    if (nit != nodes_.end()) {
        nit->second->erase(*pit, pit + 1, pend);
        if (nit->second->isdead())
            nodes_.erase(nit);
    }
}

void
names::do_eraserecur()
{
    clear();
}

void
names::do_erasewild()
{
    clearwild(nodes_, ARCEXCLUDE);
}

void
names::do_erasewild(path_iterator pit, path_iterator pend)
{
    path_iterator pnext(pit + 1);
    nodes::iterator nit(nodes_.begin()), nend(nodes_.end());
    while (nit != nend) {
        nit->second->erase(*pit, pnext, pend);
        if (nit->second->isdead())
            nodes_.erase(nit++);
        else
            ++nit;
    }
}

// typed_base

nodetype
names::do_type() const
{
    return ARCNAME;
}

names::~names() AUG_NOTHROW
{
}
