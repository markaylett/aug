#include "augaspp.hpp"

#include <map>

using namespace augas;
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

    struct echoserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            const char* serv = augas::getenv("service.echo.serv");
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
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            //setsslserver(sock, "server");
            sock.setuser(new string());
            send(sock, "hello\r\n", 7);
            setrwtimer(sock, 15000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t len)
        {
            string& tok(*sock.user<string>());
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + len, tok, '\n',
                     eachline(sock));
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            writelog(AUGAS_LOGINFO, "no data received for 15 seconds");
            shutdown(sock);
        }
        bool
        do_authcert(const object& sock, const char* subject,
                    const char* issuer)
        {
            augas_writelog(AUGAS_LOGINFO, "checking subject...");
            return true;
        }
        static serv_base*
        create(const char* sname)
        {
            return new echoserv();
        }
    };

    typedef basic_module<basic_factory<echoserv> > module;
}

AUGAS_MODULE(module::init, module::term)
