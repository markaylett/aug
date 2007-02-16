#include "augaspp.hpp"

using namespace augas;

namespace {

    struct serv : stubserv {
        bool
        do_start(const char* sname)
        {
            writelog(AUGAS_LOGINFO, "starting...");
            tcplisten(sname, "0.0.0.0", "5000");
            return true;
        }
        void
        do_data(const object& sock, const char* buf, size_t size)
        {
            send(sock, buf, size);
        }
    };

    typedef basic_module<basic_factory<serv> > module;
}

AUGAS_MODULE(module::init, module::term)
