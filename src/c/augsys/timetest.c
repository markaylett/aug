/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>
#include <stdlib.h> /* exit() */

static void
test(void)
{
    aug_clock* clock;
    long tz;
    time_t in, out;
    struct tm tm;
    time(&in);

    clock = aug_getclock(aug_tlx);
    tz = aug_gettimezone(clock);
    aug_release(clock);

    aug_ctxinfo(aug_tlx, "timezone=[%ld]", tz);

    if (!aug_gmtime(&in, &tm)) {
        aug_perrinfo(aug_tlx, "aug_gmtime() failed", NULL);
        exit(1);
    }

    if (-1 == (out = aug_timegm(&tm))) {
        aug_perrinfo(aug_tlx, "aug_timegm() failed", NULL);
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
    if (aug_initbasictlx() < 0)
        return 1;
    test();
    return 0;
}
