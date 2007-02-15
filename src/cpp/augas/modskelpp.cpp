#include "augaspp.hpp"

using namespace augas;

namespace {

    struct serv : public serv_base {
        explicit
        serv(const char* name)
        {
        }
    };

    struct factory {
        explicit
        factory(const char* name)
        {
        }
        serv_base*
        create(const char* name)
        {
            return new serv(name);
        }
    };

    typedef basic_module<factory> module;
}

AUGAS_MODULE(module::init, module::term)
