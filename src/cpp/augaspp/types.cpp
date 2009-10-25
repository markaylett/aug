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
#include "augaspp/types.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

using namespace aug;
using namespace std;

types::~types() AUG_NOTHROW
{
}

void
types::insert(unsigned id, const std::string& name)
{
    byid_.insert(make_pair(id, name));
    byname_.insert(make_pair(name, id));
}

bool
types::getbyid(unsigned id, string& name) const
{
    const map<unsigned, string>::const_iterator it(byid_.find(id));
    if (it == byid_.end())
        return false;
    name = it->second;
    return true;
}

bool
types::getbyname(const string& name, unsigned& id) const
{
    const map<string, unsigned>::const_iterator it(byname_.find(name));
    if (it == byname_.end())
        return false;
    id = it->second;
    return true;
}
