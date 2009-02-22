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
#include "augmodpp.hpp"

#include "augctxpp/mpool.hpp"
#include "augutilpp/string.hpp"

#include <map>

namespace mod = aug::mod;

using namespace mod;
using namespace std;

namespace {

    struct eachline : aug::mpool_ops {
        handle sock_;
        explicit
        eachline(const handle& sock)
            : sock_(sock)
        {
        }
        void
        operator ()(std::string& tok)
        {
            aug::trim(tok);
            transform(tok.begin(), tok.end(), tok.begin(), aug::ucase);
            tok += "\r\n";

            send(sock_, tok.c_str(), tok.size());
        }
    };

    struct echo : basic_session, aug::mpool_ops {
        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting...");
            const char* serv = mod::getenv("session.echo.serv");
            const char* sslctx = mod::getenv("session.echo.sslcontext", 0);
            if (!serv)
                return false;
            if (sslctx)
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);

            tcplisten("0.0.0.0", serv, sslctx);
            return true;
        }
        void
        do_closed(const handle& sock)
        {
            delete sock.user<string>();
        }
        bool
        do_accepted(handle& sock, const char* name)
        {
            sock.setuser(new string());
            setrwtimer(sock, 15000, MOD_TIMRD);
            return true;
        }
        bool
        do_auth(const handle& sock, const char* subject, const char* issuer)
        {
            mod_writelog(MOD_LOGINFO, "checking subject...");
            return true;
        }
        void
        do_recv(const handle& sock, const void* buf, size_t len)
        {
            string& tok(*sock.user<string>());
            try {
                tokenise(static_cast<const char*>(buf),
                         static_cast<const char*>(buf) + len, tok, '\n',
                         eachline(sock));
            } catch (...) {
                mod_writelog(MOD_LOGINFO, "shutting now...");
                shutdown(sock, 1);
                throw;
            }
        }
        void
        do_error(const handle& sock, const char* desc)
        {
            writelog(MOD_LOGERROR, "server error: %s", desc);
        }
        void
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            writelog(MOD_LOGINFO, "no data received for 15 seconds");
            shutdown(sock, 0);
        }
        static session_base*
        create(const char* sname)
        {
            return new (aug::tlx) echo();
        }

        ~echo() MOD_NOTHROW
        {
        }
    };

    typedef basic_module<basic_factory<echo> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
