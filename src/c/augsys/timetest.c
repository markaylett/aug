/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>
#include <stdlib.h> /* exit() */

static void
test(void)
{
    time_t in, out;
    struct tm tm;
    time(&in);

    aug_info("timezone=[%ld]", aug_timezone());

    if (!aug_gmtime(&in, &tm)) {
        aug_perrinfo(aug_tlerr, "aug_gmtime() failed");
        exit(1);
    }

    if (-1 == (out = aug_timegm(&tm))) {
        aug_perrinfo(aug_tlerr, "aug_timegm() failed");
        exit(1);
    }

    if (in != out) {
        fprintf(stderr, "time mismatch: in=[%ld], out=[%ld]\n",
                (long)in, (long)out);
        exit(1);
    }
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    aug_atexitinit(&errinfo);
    test();
    return 0;
}
