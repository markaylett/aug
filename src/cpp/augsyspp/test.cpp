/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
#include "augsyspp/base.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/log.h"

#include <fcntl.h>
#include <unistd.h>

using namespace aug;
using namespace std;

namespace {

    smartfd
    open()
    {
        int fd(::open("/dev/null", O_RDONLY));
        if (-1 != fd)
            try {

                openfd(fd, AUG_FDFILE);
                return smartfd::attach(fd);
            } catch (...) {
                close(fd);
                throw;
            }
        return null;
    }

    void
    hook_(int fd, int type, void* data)
    {
        aug_info("closing type: %d", type);
    }
}

int
main(int argc, char* argv[])
{
    try {

        initialiser init;
        smartfd sfd1(open());
        setfdhook(sfd1, hook_, 0);

        openfd(101, AUG_FDUSER);
        smartfd sfd2(smartfd::attach(101));
        setfdhook(sfd2, hook_, 0);

    } catch (const exception& e) {

        aug_error("%s", e.what());
        return 1;
    }

    return 0;
}
