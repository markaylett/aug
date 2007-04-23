#include "augaspp.hpp"

#include "augsyspp.hpp"

using namespace augas;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    const void*
    buf(void* arg, size_t* size)
    {
        if (size)
            *size = sizeof(MSG) - 1;
        return MSG;
    }

    const struct augas_vartype vartype = {
        NULL,
        buf
    };

    const struct augas_var var = {
        &vartype,
        NULL
    };

    void
    dosend(object& sock)
    {
        send(sock, MSG, sizeof(MSG) - 1);
    }

    void
    dosendv(object& sock)
    {
        sendv(sock, var);
    }

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
        void (*fn_)(object&);
        object sock_;
        explicit
        eachline(void (*fn)(object&), const object& sock)
            : fn_(fn),
              sock_(sock)
        {
        }
        void
        operator ()(std::string& tok)
        {
            state& s(*sock_.user<state>());
            if (0 == --s.torecv_)
                shutdown(sock_);
            else if (0 < s.tosend_--)
                fn_(sock_);
        }
    };

    struct benchserv : basic_serv {
        void (*send_)(object&);
        unsigned conns_, estab_, echos_;
        size_t bytes_;
        timeval start_;
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");

            if (atoi(augas::getenv("service.bench.sendv", "1"))) {
                send_ = dosendv;
                augas_writelog(AUGAS_LOGINFO, "sendv: yes");
            } else {
                send_ = dosend;
                augas_writelog(AUGAS_LOGINFO, "sendv: no");
            }

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
            if (0 < --estab_) {
                augas_writelog(AUGAS_LOGINFO, "%d established", estab_);
                return;
            }

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

            stopall();
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
            //setsslclient(sock, "client");
            state& s(*sock.user<state>());
            send_(sock);
            --s.tosend_;
        }
        void
        do_data(const object& sock, const void* buf, size_t len)
        {
            bytes_ += len;
            state& s(*sock.user<state>());
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + len, s.tok_, '\n',
                     eachline(send_, sock));
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
