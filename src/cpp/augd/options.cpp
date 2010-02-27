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
#define AUGD_BUILD
#include "augd/options.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augd/exception.hpp"

#include "augutilpp/file.hpp"

using namespace aug;
using namespace augd;
using namespace std;

namespace {
    aug_result
    cb(const char* name, const char* value, void* arg)
    {
        map<string, string>& x(*static_cast<map<string, string>*>(arg));
        x[name] = value;
        return 0;
    }
}

options::~options() AUG_NOTHROW
{
}

void
options::read(const char* path)
{
    options_.clear();
    readconf(path, confcb<cb>, &options_);
}

void
options::set(const string& name, const string& value)
{
    options_[name] = value;
}

const char*
options::get(const string& name, const char* def) const
{
    map<string, string>::const_iterator it(options_.find(name));
    if (options_.find(name) == options_.end())
        return def;
    return it->second.c_str();
}

const string&
options::get(const string& name) const
{
    map<string, string>::const_iterator it(options_.find(name));
    if (options_.find(name) == options_.end())
        throw augd_error(__FILE__, __LINE__, ECONFIG,
                         "missing option: name=[%s]", name.c_str());
    return it->second;
}
