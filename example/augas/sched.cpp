#include "augaspp.hpp"

using namespace augas;
using namespace std;

namespace {
    struct schedserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            return true;
        }
        static serv_base*
        create(const char* sname)
        {
            return new schedserv();
        }
    };

    typedef basic_module<basic_factory<schedserv> > module;
}

AUGAS_MODULE(module::init, module::term)
