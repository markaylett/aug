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
#include "augsubpp/recur.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsubpp/utility.hpp"

using namespace aug;
using namespace std;

// node_base

void
recur::do_clear()
{
    match_ = ARCNOTHING;
    exnames_.clear();
    exwild_.clear();
}

unsigned
recur::do_depth() const
{
    return AUG_MAX(exdepth(exnames_), exdepth(exwild_));
}

bool
recur::do_ismatch(path_iterator pit, path_iterator pend) const
{
    const match m(do_query(pit, pend));
    return ARCTRANSIT == m || ARCINCLUDE == m;
}

void
recur::do_print(const string& path, ostream& os) const
{
    os << path << "/-\n";
    exnames_.print(path, os);
    exwild_.print(path, os);
}

// query_base

match
recur::do_query(path_iterator pit, path_iterator pend) const
{
    // Match is checked for greater efficiency.

    if (ARCEXCLUDE == match_
        || ARCINCLUDE == exnames_.query(pit, pend)
        || ARCINCLUDE == exwild_.query(pit, pend))
        return ARCEXCLUDE;

    return match_;
}

bool
recur::do_isdead() const
{
    return ARCINCLUDE != match_;
}

// inner_base

void
recur::do_insertname(const string& name)
{
    exnames_.insertname(name);
}

void
recur::do_insertname(const string& name, path_iterator pit,
                      path_iterator pend)
{
    exnames_.insertname(name, pit, pend);
}

void
recur::do_insertrecur()
{
    match_ = ARCINCLUDE;
    exnames_.clear();
    exwild_.clear();
}

void
recur::do_insertwild()
{
    exnames_.insertwild();
    exwild_.insertwild();
}

void
recur::do_insertwild(path_iterator pit, path_iterator pend)
{
    exnames_.insertwild(pit, pend);
    exwild_.insertwild(pit, pend);
}

void
recur::do_erasename(const string& name)
{
    exnames_.erasename(name);
}

void
recur::do_erasename(const string& name, path_iterator pit,
                     path_iterator pend)
{
    exnames_.erasename(name, pit, pend);
}

void
recur::do_eraserecur()
{
    match_ = ARCEXCLUDE;
    exnames_.clear();
    exwild_.clear();
}

void
recur::do_erasewild()
{
    exnames_.erasewild();
    exwild_.erasewild();
}

void
recur::do_erasewild(path_iterator pit, path_iterator pend)
{
    exnames_.erasewild(pit, pend);
    exwild_.erasewild(pit, pend);
}

// typed_base

nodetype
recur::do_type() const
{
    return ARCRECUR;
}

recur::~recur() AUG_NOTHROW
{
}
