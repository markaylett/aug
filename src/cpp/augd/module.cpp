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
#include "augd/module.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augd/exception.hpp"

#include "augext/log.h"

#include "augctx/string.h" // aug_strlcpy()

#include <exception>

using namespace aug;
using namespace augd;
using namespace std;

module::~module() AUG_NOTHROW
{
    try {
        AUG_CTXDEBUG2(aug_tlx, "terminating module: name=[%s]", name_);
        termfn_();
    } AUG_PERRINFOCATCH;
}

module::module(const char* name, const char* path, const mod_host& host)
    : lib_(getmpool(aug_tlx), path)
{
    aug_strlcpy(name_, name, sizeof(name_));

    AUG_CTXDEBUG2(aug_tlx, "resolving symbols in module: name=[%s]", name_);

    mod_initfn initfn(dlsym<mod_initfn>(lib_, "mod_init"));
    termfn_ = dlsym<mod_termfn>(lib_, "mod_term");
    createfn_ = dlsym<mod_createfn>(lib_, "mod_create");

    AUG_CTXDEBUG2(aug_tlx, "initialising module: name=[%s]", name_);

    if (!initfn(name_, &host))
        throw augd_error(__FILE__, __LINE__, EMODCALL, "mod_init() failed");
}

mod::sessionptr
module::create(const char* name) AUG_NOTHROW
{
    return object_attach<mod_session>(createfn_(name));
}
