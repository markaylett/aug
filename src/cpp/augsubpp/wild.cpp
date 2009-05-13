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
#include "augsubpp/wild.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsubpp/utility.hpp"

using namespace aug;
using namespace std;

// node_base

void
wild::do_clear()
{
    match_ = SUBNOTHING;
    wildchild_->clear();
    exnames_.clear();
}

unsigned
wild::do_depth() const
{
    return wildchild_->depth();
}

bool
wild::do_ismatch(path_iterator pit, path_iterator pend) const
{
    const match m(do_query(pit, pend));
    return SUBTRANSIT == m || SUBINCLUDE == m;
}

void
wild::do_print(const string& path, ostream& os) const
{
    if (SUBINCLUDE == match_)
        os << path << "/*\n";
    else if (SUBEXCLUDE == match_)
        os << '!' << path << "/*\n";
    wildchild_->print(path + "/*", os);
    exnames_.print(path, os);
}

// query_base

match
wild::do_query(path_iterator pit, path_iterator pend) const
{
    if (SUBINCLUDE == exnames_.query(pit, pend))
        return SUBEXCLUDE;

    ++pit;
    return pit == pend ? match_ : wildchild_->query(pit, pend);
}

bool
wild::do_isdead() const
{
    return SUBINCLUDE != match_ && wildchild_->isdead();
}

// inner_base

void
wild::do_insertname(const string& name)
{
    // Wildcard exception.
    exnames_.insertname(name);
}

void
wild::do_insertname(const string& name, path_iterator pit,
                     path_iterator pend)
{
    // Wildcard exception.
    exnames_.insertname(name, pit, pend);
}

void
wild::do_insertrecur()
{
    clear();
}

void
wild::do_insertwild()
{
    match_ = SUBINCLUDE;
    exnames_.insertwild();
}

void
wild::do_insertwild(path_iterator pit, path_iterator pend)
{
    if (SUBINCLUDE != match_)
        match_ = SUBTRANSIT;
    insertnode(*pit, pit + 1, pend, *wildchild_);
    exnames_.insertwild(pit, pend);
}

void
wild::do_erasename(const string& name)
{
    exnames_.erasename(name);
}

void
wild::do_erasename(const string& name, path_iterator pit,
                    path_iterator pend)
{
    exnames_.erasename(name, pit, pend);
}

void
wild::do_eraserecur()
{
    clear();
}

void
wild::do_erasewild()
{
    match_ = SUBEXCLUDE;
    exnames_.erasewild();
}

void
wild::do_erasewild(path_iterator pit, path_iterator pend)
{
    erasenode(*pit, pit + 1, pend, *wildchild_);
    exnames_.erasewild(pit, pend);
}

// typed_base

nodetype
wild::do_type() const
{
    return SUBWILD;
}

wild::~wild() AUG_NOTHROW
{
}
