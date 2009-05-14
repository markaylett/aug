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
#define AUGARCPP_BUILD
#include "augarcpp/utility.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augarcpp/exception.hpp"
#include "augarcpp/inner.hpp"

#include "augctx/defs.h"        // AUG_MAX()

using namespace aug;
using namespace std;

namespace {

    const char RECUR_[] = "-";
    const char WILD_[] = "*";
}

AUGARCPP_API unsigned
aug::exdepth(const node_base& node)
{
    return AUG_MAX(1U, node.depth()) - 1;
}

AUGARCPP_API match
aug::foldmatch(match l, match r)
{
    switch (l) {
    case ARCNOTHING:
        l = r;
        break;
    case ARCTRANSIT:
        if (ARCINCLUDE == r || ARCEXCLUDE == r)
            l = r;
        break;
    case ARCINCLUDE:
        if (ARCEXCLUDE == r)
            l = r;
        break;
    case ARCEXCLUDE:
        break;
    }
    return l;
}

AUGARCPP_API nodetype
aug::tonodetype(const string& value)
{
    nodetype type(ARCNAME);

    if (value == RECUR_)
        type = ARCRECUR;
    else if (value == WILD_)
        type = ARCWILD;

    return type;
}

AUGARCPP_API void
aug::erasenode(const string& head, path_iterator pit, path_iterator pend,
               inner_base& node)
{
    switch (tonodetype(head)) {
    case ARCNAME:
        if (pit == pend)
            node.erasename(head);
        else
            node.erasename(head, pit, pend);
        break;
    case ARCRECUR:
        if (pit == pend)
            node.eraserecur();
        else
            throw invalid_path();
        break;
    case ARCWILD:
        if (pit == pend)
            node.erasewild();
        else
            node.erasewild(pit, pend);
        break;
    }
}

AUGARCPP_API void
aug::erasenode(const string& s, inner_base& node)
{
    const path p(splitpath(s));
    if (!p.empty())
        erasenode(p[0], p.begin() + 1, p.end(), node);
}

AUGARCPP_API void
aug::insertnode(const string& head, path_iterator pit, path_iterator pend,
                inner_base& node)
{
    switch (tonodetype(head)) {
    case ARCNAME:
        if (pit == pend)
            node.insertname(head);
        else
            node.insertname(head, pit, pend);
        break;
    case ARCRECUR:
        if (pit == pend)
            node.insertrecur();
        else
            throw invalid_path();
        break;
    case ARCWILD:
        if (pit == pend)
            node.insertwild();
        else
            node.insertwild(pit, pend);
        break;
    }
}

AUGARCPP_API void
aug::insertnode(const string& s, inner_base& node)
{
    const path p(splitpath(s));
    if (!p.empty())
        insertnode(p[0], p.begin() + 1, p.end(), node);
}
