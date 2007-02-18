#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {

    struct state {
        string head_;
    };

    struct serv : basic_serv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            tcplisten(sname, "0.0.0.0", "5000");
            return true;
        }
        void
        do_closed(const object& sock)
        {
            delete sock.user<state>();
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            sock.setuser(new state());
            send(sock, "hello\r\n", 7);
            setrwtimer(sock, 15000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
            state* s(sock.user<state>());

            string tail(buf, size);
            while (shift(s->head_, tail)) {

                if (!s->head_.empty()
                    && '\r' == s->head_[s->head_.size() - 1])
                    s->head_.resize(s->head_.size() - 1);

                s->head_ = urlencode(s->head_);

                s->head_ += "\r\n";
                send(sock, s->head_.c_str(), s->head_.size());
                s->head_.clear();
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
