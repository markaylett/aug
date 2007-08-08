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

    aug_clock_t clck;
    double start, stop;

    aug_atexitinit(&errinfo);

    if (!(clck = aug_createclock())) {
        aug_perrinfo(NULL, "aug_createclock() failed");
        return 1;
    }

    if (!aug_elapsed(clck, &start)) {
        aug_perrinfo(NULL, "aug_elapsed() failed");
        return 1;
    }

    /* Sleep for 2 ms. */

    aug_msleep(2);

    /* 1ms tollerance. */

    if (!aug_elapsed(clck, &stop)) {
        aug_perrinfo(NULL, "aug_elapsed() failed");
        return 1;
    }

    if (0.001 < stop - start && stop - start < 0.003) {
        aug_error("unexpected interval: start=%0.6f, stop=%0.6f",
                  start, stop);
        return 1;
    }

    if (-1 == aug_destroyclock(clck)) {
        aug_perrinfo(NULL, "aug_destroyclock() failed");
        return 1;
    }

    return 0;
}
