#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {

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
            string& head(*sock.user<string>());

            string tail(buf, size);
            while (shift(head, tail)) {

                if (!head.empty() && '\r' == head[head.size() - 1])
                    head.resize(head.size() - 1);

                head = urlencode(head);

                head += "\r\n";
                send(sock, head.c_str(), head.size());
                head.clear();
            }
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
