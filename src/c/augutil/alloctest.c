/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augutil.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    int i;
    aug_atexitinit(&errinfo);

    for (i = 0; i < 5; ++i) {
        void* x = aug_allocsmall(16);
        aug_freesmall(x);
    }

    return 0;
}
