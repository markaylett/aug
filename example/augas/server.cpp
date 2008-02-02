/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MOD_BUILD
#include "augmodpp.hpp"

#include <map>

namespace mod = aug::mod;

using namespace mod;
using namespace std;

namespace {

    struct eachline {
        handle sock_;
        explicit
        eachline(const handle& sock)
            : sock_(sock)
        {
        }
        void
        operator ()(std::string& tok)
        {
            trim(tok);
            transform(tok.begin(), tok.end(), tok.begin(), ucase);
            tok += "\r\n";

            send(sock_, tok.c_str(), tok.size());
        }
    };

    struct echo : basic_session {
        bool
        do_start(const char* sname)
        {
            writelog(MOD_LOGINFO, "starting...");
            const char* serv = mod::getenv("session.echo.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_closed(const handle& sock)
        {
            delete sock.user<string>();
        }
        bool
        do_accepted(handle& sock, const char* addr, unsigned short port)
        {
            const char* sslctx = mod::getenv("session.echo.sslcontext", 0);
            if (sslctx) {
                writelog(MOD_LOGINFO, "sslcontext: %s", sslctx);
                setsslserver(sock, sslctx);
            }
            sock.setuser(new string());
            setrwtimer(sock, 15000, MOD_TIMRD);
            return true;
        }
        void
        do_data(const handle& sock, const void* buf, size_t len)
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
        do_rdexpire(const handle& sock, unsigned& ms)
        {
            writelog(MOD_LOGINFO, "no data received for 15 seconds");
            shutdown(sock, 0);
        }
        bool
        do_authcert(const handle& sock, const char* subject,
                    const char* issuer)
        {
            mod_writelog(MOD_LOGINFO, "checking subject...");
            return true;
        }
        static session_base*
        create(const char* sname)
        {
            return new echo();
        }
    };

    typedef basic_module<basic_factory<echo> > module;
}

MOD_ENTRYPOINTS(module::init, module::term)
