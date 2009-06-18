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
#include "augutilpp/object.hpp"
#include "augutilpp/string.hpp"

#include "augext/boxptr.h"

#include <map>

using namespace mod;
using namespace std;

namespace {

    struct eachline : aug::mpool_ops {
        const mod_handle& sock_;
        explicit
        eachline(const mod_handle& sock)
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

    class token : public aug::boxptr_base<token>, public aug::mpool_ops {
        string impl_;
    public:
        ~token() AUG_NOTHROW
        {
            // Deleted from base.
        }
        void*
        unboxptr_() AUG_NOTHROW
        {
            return &impl_;
        }
        static aug::boxptrptr
        create()
        {
            return attach(new (aug::tlx) token());
        }
    };

    class echo : public basic_session<echo>, public aug::mpool_ops {
        const string sname_;
        explicit
        echo(const string& sname)
            : sname_(sname)
        {
        }
    public:
        ~echo() AUG_NOTHROW
        {
            // Deleted from base.
        }
        mod_bool
        start()
        {
            writelog(MOD_LOGINFO, "starting...");
            const char* serv(mod::getenv("session.echo.serv"));
            const char* sslctx(mod::getenv("session.echo.sslcontext"));
            if (!serv)
                return MOD_FALSE;
            if (sslctx)
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);

            tcplisten("0.0.0.0", serv, sslctx);
            return MOD_TRUE;
        }
        void
        stop()
        {
        }
        void
        reconf()
        {
        }
        void
        event(const char* from, const char* type, mod_id id,
              aug::objectref ob)
        {
        }
        void
        closed(mod_handle& sock)
        {
            aug_assign(sock.ob_, 0);
        }
        void
        teardown(mod_handle& sock)
        {
            mod::shutdown(sock, 0);
        }
        mod_bool
        accepted(mod_handle& sock, const char* name)
        {
            aug::boxptrptr bp(token::create());
            aug_assign(sock.ob_, bp.base());

            setrwtimer(sock, 15000, MOD_TIMRD);
            return MOD_TRUE;
        }
        void
        connected(mod_handle& sock, const char* name)
        {
        }
        mod_bool
        auth(mod_handle& sock, const char* subject, const char* issuer)
        {
            return MOD_TRUE;
        }
        void
        recv(mod_handle& sock, const void* buf, size_t len)
        {
            string* tok(aug::obtop<string>(sock.ob_));
            try {
                tokenise(static_cast<const char*>(buf),
                         static_cast<const char*>(buf) + len, *tok, '\n',
                         eachline(sock));
            } catch (...) {
                writelog(MOD_LOGINFO, "shutting now...");
                shutdown(sock, 1);
                throw;
            }
        }
        void
        error(mod_handle& sock, const char* desc)
        {
            writelog(MOD_LOGERROR, "server error: %s", desc);
        }
        void
        rdexpire(mod_handle& sock, unsigned& ms)
        {
            writelog(MOD_LOGINFO, "no data received for 15 seconds");
            shutdown(sock, 0);
        }
        void
        wrexpire(mod_handle& sock, unsigned& ms)
        {
        }
        void
        expire(mod_handle& timer, unsigned& ms)
        {
        }
        static sessionptr
        create(const char* sname)
        {
            return attach(new (aug::tlx) echo(sname));
        }
    };

    typedef basic_module<basic_factory<echo> > module;
}

MOD_ENTRYPOINTS(module::init, module::term, module::create)
