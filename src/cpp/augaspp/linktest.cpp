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
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augaspp/session.hpp"

#include "augctxpp.hpp"

#include <iostream>

using namespace aug;
using namespace std;

namespace {

    class session : public session_base, public mpool_ops {

        const char*
        do_name() const AUG_NOTHROW
        {
            return "test";
        }

        mod_bool
        do_active() const AUG_NOTHROW
        {
            return AUG_TRUE;
        }

        mod_bool
        do_start() AUG_NOTHROW
        {
            return AUG_TRUE;
        }

        void
        do_reconf() const AUG_NOTHROW
        {
        }

        void
        do_event(const char* from, const char* type, mod_id id,
                 objectref ob) const AUG_NOTHROW
        {
        }

        void
        do_closed(mod_handle& sock) const AUG_NOTHROW
        {
        }

        void
        do_teardown(mod_handle& sock) const AUG_NOTHROW
        {
        }

        mod_bool
        do_accepted(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
            return AUG_TRUE;
        }

        void
        do_connected(mod_handle& sock, const char* name) const AUG_NOTHROW
        {
        }

        mod_bool
        do_auth(mod_handle& sock, const char* subject,
                const char* issuer) const AUG_NOTHROW
        {
            return AUG_TRUE;
        }

        void
        do_recv(mod_handle& sock, const char* buf,
                size_t size) const AUG_NOTHROW
        {
        }

        void
        do_mrecv(const char* node, unsigned sess, unsigned short type,
                 const void* buf, size_t len) const AUG_NOTHROW
        {
        }

        void
        do_error(mod_handle& sock, const char* desc) const AUG_NOTHROW
        {
        }

        void
        do_rdexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
        }

        void
        do_wrexpire(mod_handle& sock, unsigned& ms) const AUG_NOTHROW
        {
        }

        void
        do_expire(mod_handle& timer, unsigned& ms) const AUG_NOTHROW
        {
        }

    public:
        ~session() AUG_NOTHROW
        {
        }
    };
}

int
main(int argc, char* argv[])
{
    try {

        // Initialise aug libraries.

        scoped_init init(tlx);

        /*
          Info: resolving vtable for aug::session_base by linking to
          __imp___ZTVN3aug12session_baseE
          (auto-importc:/usr/mingw/bin/../lib/gcc/mingw32/4.4.1/../../../../mingw32/bin/ld.exe:
          warning: auto-importing has been activated without
          --enable-auto-import specified on the command line.

          This should work unless it involves constant data structures
          referencing symbols from auto-imported DLLs.)
        */

        session s;
        return 0;

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
