/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsyspp/base.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/errinfo.h"
#include "augsys/log.h"

#include <fcntl.h>
#include <unistd.h>

using namespace aug;
using namespace std;

namespace {

    int
    close_(int fd)
    {
        aug_info("closing fd: %d", (int)fd);
        return aug_posixdriver()->close_(fd);
    }

    struct aug_driver driver_ = {
        close_, 0, 0, 0, 0, 0
    };

    smartfd
    open()
    {
        int fd(::open("Makefile", O_RDONLY));
        if (-1 != fd)
            try {
                openfd(fd, &driver_);
                return smartfd::attach(fd);
            } catch (...) {
                close(fd);
                throw;
            }
        return null;
    }
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);

    try {

        aug_extdriver(&driver_, 0);
        smartfd sfd1(open());
        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
