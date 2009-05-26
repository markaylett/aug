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
#include "augarcpp/node.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutilpp/string.hpp"

#include "augctxpp/mpool.hpp"

#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    class nodes {
        path* path_;
    public:
        explicit
        nodes(path& p)
            : path_(&p)
        {
        }
        void
        operator ()(string& name)
        {
            if (name == "..") {
                if (path_->empty())
                    throw underflow_error("path underflow");
                path_->pop_back();
            } else if (!name.empty() && name != ".")
                path_->push_back(name);
        }
    };
}

AUGARCPP_API path
aug::splitpath(const string& s)
{
    path p;
    splitn(s.begin(), s.end(), '/', nodes(p));
    return p;
}

AUGARCPP_API
node_base::~node_base() AUG_NOTHROW
{
}
