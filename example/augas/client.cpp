#include "augaspp.hpp"

#include "augsyspp.hpp"

using namespace augas;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    struct state {
        string tok_;
        unsigned tosend_, torecv_;
        explicit
        state(int echos)
            : tosend_(echos),
              torecv_(echos)
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
            if (0 == --s.torecv_)
                shutdown(sock_);
            else if (0 < s.tosend_--)
                send(sock_, MSG, sizeof(MSG) - 1);
        }
    };

    struct benchserv : basic_serv {
        unsigned conns_, estab_, echos_;
        size_t bytes_;
        timeval start_;
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");

            const char* serv = augas::getenv("service.bench.serv");
            if (!serv)
                return false;

            const char* host = augas::getenv("service.bench.host",
                                             "localhost");
            conns_ = atoi(augas::getenv("service.bench.conns", "100"));
            echos_ = atoi(augas::getenv("service.bench.echos", "1000"));

            augas_writelog(AUGAS_LOGINFO, "host: %s", host);
            augas_writelog(AUGAS_LOGINFO, "serv: %s", serv);
            augas_writelog(AUGAS_LOGINFO, "conns: %d", conns_);
            augas_writelog(AUGAS_LOGINFO, "echos: %d", echos_);

            aug::gettimeofday(start_);

            for (; estab_ < conns_; ++estab_)
                tcpconnect(host, serv, new state(echos_));
            return true;
        }
        void
        do_closed(const object& sock)
        {
            delete sock.user<state>();
            if (0 < --estab_)
                return;

            timeval tv;
            aug::gettimeofday(tv);
            aug::tvsub(tv, start_);

            double ms(static_cast<double>(aug::tvtoms(tv)));

            augas_writelog(AUGAS_LOGINFO, "total time: %f ms", ms);

            ms /= static_cast<double>(conns_);
            augas_writelog(AUGAS_LOGINFO, "time per conn: %f ms", ms);

            ms /= static_cast<double>(echos_);
            augas_writelog(AUGAS_LOGINFO, "echos per sec: %f", 1000.0 / ms);

            double k(static_cast<double>(bytes_) / 1024);
            augas_writelog(AUGAS_LOGINFO, "total size: %f k", k);

            k /= static_cast<double>(conns_);
            augas_writelog(AUGAS_LOGINFO, "size per conn: %f k", k);

            ms /= static_cast<double>(echos_);
            augas_writelog(AUGAS_LOGINFO, "size per sec: %f k", 1000.0 / k);

            stopall();
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
            state& s(*sock.user<state>());
            send(sock, MSG, sizeof(MSG) - 1);
            --s.tosend_;
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
            bytes_ += size;
            state& s(*sock.user<state>());
            tokenise(buf, buf + size, s.tok_, '\n', eachline(sock));
        }
        benchserv()
            : conns_(0),
              estab_(0),
              bytes_(0)
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
