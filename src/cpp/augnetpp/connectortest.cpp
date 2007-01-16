/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp/connector.hpp"

#include "augsyspp/base.hpp"
#include "augsyspp/endpoint.hpp"
#include "augsyspp/unistd.hpp"

#include "augsys/log.h"

using namespace aug;

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {
        connector ctor("127.0.0.1", "10000");
        endpoint ep(null);
        tryconnect(ctor, ep);
        tryconnect(ctor, ep);
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
