/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augsys.h"
#include "augctx.h"
#include "augext.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_hires_t hires;
    double start, stop;

    if (aug_autobasictlx() < 0)
        return 1;

    mpool = aug_getmpool(aug_tlx);
    hires = aug_createhires(mpool);
    aug_release(mpool);

    if (!hires) {
        aug_perrinfo(aug_tlx, "aug_createhires() failed", NULL);
        return 1;
    }

    if (!aug_elapsed(hires, &start)) {
        aug_perrinfo(aug_tlx, "aug_elapsed() failed", NULL);
        return 1;
    }

    /* Sleep for 500 ms. */

    aug_msleep(500);

    if (!aug_elapsed(hires, &stop)) {
        aug_perrinfo(aug_tlx, "aug_elapsed() failed", NULL);
        return 1;
    }

    /* Allow 20ms tollerance. */

    stop -= start;
    if (stop < 0.48 || 0.52 < stop) {
        aug_ctxerror(aug_tlx, "unexpected interval: stop=%0.6f", stop);
        return 1;
    }

    aug_destroyhires(hires);
    return 0;
}
