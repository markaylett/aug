/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    if (-1 == aug_atexitinit(&errinfo)) {
        aug_perror("aug_atexitinit() failed");
        return 1;
    }

    return 0;
}
