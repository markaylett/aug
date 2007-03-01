#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    struct state {
        string tok_;
        unsigned sent_, recvd_;
        state()
            : sent_(0),
              recvd_(0)
        {
        }
    };

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
            state& s(*sock_.user<state>());
            if (1000 == ++s.recvd_)
                shutdown(sock_);
            else if (s.sent_ < 1000) {
                send(sock_, MSG, sizeof(MSG) - 1);
                ++s.sent_;
            }
        }
    };

    struct benchserv : basic_serv {
        unsigned conns_;
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            const char* host = augas::getenv("service.bench.host");
            if (!host)
                host = "localhost";
            const char* serv = augas::getenv("service.bench.serv");
            if (!serv)
                return false;
            for (; conns_ < 100; ++conns_)
                tcpconnect(host, serv, new state());
            return true;
        }
        void
        do_closed(const object& sock)
        {
            delete sock.user<state>();
            if (0 == --conns_)
                stopall();
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
            state& s(*sock.user<state>());
            send(sock, MSG, sizeof(MSG) - 1);
            ++s.sent_;
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
            state& s(*sock.user<state>());
            tokenise(buf, buf + size, s.tok_, '\n', eachline(sock));
        }
        benchserv()
            : conns_(0)
        {
        }
        static serv_base*
        create(const char* sname)
        {
            return new benchserv();
        }
    };

    typedef basic_module<basic_factory<benchserv> > module;
}

AUGAS_MODULE(module::init, module::term)
