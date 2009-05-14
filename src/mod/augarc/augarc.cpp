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
#define MOD_BUILD
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmodpp.hpp"

#include "augutilpp/string.hpp"
#include "augctxpp/mpool.hpp"

namespace mod = aug::mod;

using namespace mod;
using namespace std;

namespace {

    struct echoline {
        handle sock_;
        explicit
        echoline(const handle& sock)
            : sock_(sock)
        {
        }
        void
        operator ()(string& line)
        {
            aug::trim(line);
            transform(line.begin(), line.end(), line.begin(),
                      aug::ucase);
            line += "\r\n";
            send(sock_, line.c_str(), line.size());
        }
    };

    struct session : basic_session, aug::mpool_ops {
        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting session [%s]", sname);
            const char* serv(mod::getenv("session.arc.serv"));
            if (!serv)
                return false;
            tcplisten("0.0.0.0", serv);
            return true;
        }
        bool
        do_accepted(handle& sock, const char* name)
        {
            sock.setuser(new string());
            send(sock, "HELLO\r\n", 7);
            setrwtimer(sock, 15000, MOD_TIMRD);
            return true;
        }
        void
        do_closed(const handle&sock)
        {
            delete sock.user<string>();
        }
        void
        do_recv(const handle& sock, const void* buf, size_t size)
        {
            string& tok(*sock.user<string>());
            aug::tokenise(static_cast<const char*>(buf),
                          static_cast<const char*>(buf) + size, tok, '\n',
                          echoline(sock));
        }
        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            shutdown(sock, 0);
        }
        ~session() AUG_NOTHROW
        {
        }
        static session_base*
        create(const char* sname)
        {
            return new (aug::tlx) session();
        }
    };

    typedef basic_module<basic_factory<session> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
