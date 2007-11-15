/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMOD_BUILD
#include "maudpp.hpp"

#include <map>

using namespace maud;
using namespace std;

namespace {

    struct eachline {
        object sock_;
        explicit
        eachline(const object& sock)
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

    struct echosession : basic_session {
        bool
        do_start(const char* sname)
        {
            writelog(MAUD_LOGINFO, "starting...");
            const char* serv = maud::getenv("session.echo.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_closed(const object& sock)
        {
            delete sock.user<string>();
        }
        bool
        do_accepted(object& sock, const char* addr, unsigned short port)
        {
            const char* sslctx = maud::getenv("session.echo.sslcontext", 0);
            if (sslctx) {
                writelog(MAUD_LOGINFO, "sslcontext: %s", sslctx);
                setsslserver(sock, sslctx);
            }
            sock.setuser(new string());
            setrwtimer(sock, 15000, MAUD_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t len)
        {
            string& tok(*sock.user<string>());
            try {
                tokenise(static_cast<const char*>(buf),
                         static_cast<const char*>(buf) + len, tok, '\n',
                         eachline(sock));
            } catch (...) {
                maud_writelog(MAUD_LOGINFO, "shutting now...");
                shutdown(sock, 1);
                throw;
            }
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            writelog(MAUD_LOGINFO, "no data received for 15 seconds");
            shutdown(sock, 0);
        }
        bool
        do_authcert(const object& sock, const char* subject,
                    const char* issuer)
        {
            maud_writelog(MAUD_LOGINFO, "checking subject...");
            return true;
        }
        static session_base*
        create(const char* sname)
        {
            return new echosession();
        }
    };

    typedef basic_module<basic_factory<echosession> > module;
}

MAUD_ENTRYPOINTS(module::init, module::term)
