/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYSPP_BUILD
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
        aug_info("closing fd: %d", fd);
        return aug_posixdriver()->close_(fd);
    }

    struct aug_fddriver driver_ = {
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
    try {

        struct aug_errinfo errinfo;
        initialiser init(errinfo);

        aug_extenddriver(&driver_, 0);
        smartfd sfd1(open());

    } catch (const exception& e) {

        aug_perrinfo("error");
        return 1;
    }

    return 0;
}
