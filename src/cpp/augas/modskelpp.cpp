#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {

    struct eachline {
        const object* const sock_;
        explicit
        eachline(const object& sock)
            : sock_(&sock)
        {
        }
        void
        operator ()(std::string& s)
        {
            if (!s.empty() && '\r' == s[s.size() - 1])
                s.resize(s.size() - 1);

            reverse(s.begin(), s.end());
            s += "\r\n";

            send(*sock_, s.c_str(), s.size());
        }
    };

    struct serv : basic_serv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            tcplisten(sname, "0.0.0.0", augas::getenv("session.echo.serv"));
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
            sock.setuser(new string());
            send(sock, "hello\r\n", 7);
            setrwtimer(sock, 15000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
            string& tok(*sock.user<string>());
            tokenise(buf, buf + size, tok, '\n', eachline(sock));
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            shutdown(sock);
        }
    };

    typedef basic_module<basic_factory<serv> > module;
}

AUGAS_MODULE(module::init, module::term)
