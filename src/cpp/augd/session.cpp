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
#include "augd/session.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys.h"

#include "augctx/string.h" // aug_strlcpy()

#include <stack>

using namespace aug;
using namespace augd;
using namespace std;

namespace {

    // A stack is maintained so that the calling session is always known.

    stack<const session_base*> stack_;

    struct scoped_frame {
        ~scoped_frame() AUG_NOTHROW
        {
            stack_.pop();
        }
        explicit
        scoped_frame(const session_base* session)
        {
            stack_.push(session);
        }
    };
}

const char*
session::do_name() const AUG_NOTHROW
{
    return name_;
}

mod_bool
session::do_active() const AUG_NOTHROW
{
    return active_;
}

mod_bool
session::do_start() AUG_NOTHROW
{
    scoped_frame frame(this);
    active_ = MOD_TRUE; // Functions may be called during initialisation.
    return active_ = mod::start(session_);
}

void
session::do_reconf() const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::reconf(session_);
}

void
session::do_event(const char* from, const char* type, mod_id id,
                  objectref ob) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::event(session_, from, type, id, ob);
}

void
session::do_closed(mod_handle& sock) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::closed(session_, sock);
}

void
session::do_teardown(mod_handle& sock) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::teardown(session_, sock);
}

mod_bool
session::do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW
{
    scoped_frame frame(this);
    return mod::accepted(session_, sock, name);
}

void
session::do_connected(mod_handle& sock, const char* name) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::connected(session_, sock, name);
}

mod_bool
session::do_auth(mod_handle& sock, const char* subject,
                 const char* issuer) const AUG_NOTHROW
{
    scoped_frame frame(this);
    return mod::auth(session_, sock, subject, issuer);
}

void
session::do_recv(mod_handle& sock, const char* buf,
                 size_t size) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::recv(session_, sock, buf, size);
}

void
session::do_mrecv(const char* node, unsigned sess, unsigned short type,
                  const void* buf, size_t len) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::mrecv(session_, node, sess, type, buf, len);
}

void
session::do_error(mod_handle& sock, const char* desc) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::error(session_, sock, desc);
}

void
session::do_rdexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::rdexpire(session_, sock, ms);
}

void
session::do_wrexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::wrexpire(session_, sock, ms);
}

void
session::do_expire(mod_handle& timer, unsigned& ms) const AUG_NOTHROW
{
    scoped_frame frame(this);
    mod::expire(session_, timer, ms);
}

session::~session() AUG_NOTHROW
{
    if (active_) {
        scoped_frame frame(this);
        mod::stop(session_); // AUG_NOTHROW
        session_ = null;
    }
}

session::session(const char* name, const moduleptr& module)
    : module_(module),
      session_(module->create(name)),
      active_(MOD_FALSE)
{
    aug_strlcpy(name_, name, sizeof(name_));
}

const session_base&
augd::getsession()
{
    assert(!stack_.empty());
    return *stack_.top();
}
