/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"
#include "augsys/time.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_clock_ {
    struct timeval start_;
};

AUGUTIL_API aug_clock_t
aug_createclock(void)
{
    aug_clock_t clck = malloc(sizeof(struct aug_clock_));
    if (!clck) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (-1 == aug_gettimeofday(&clck->start_, NULL)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        free(clck);
        return NULL;
    }

    return clck;
}

AUGUTIL_API int
aug_destroyclock(aug_clock_t clck)
{
    if (clck)
        free(clck);
    return 0;
}

AUGUTIL_API int
aug_resetclock(aug_clock_t clck)
{
    if (-1 == aug_gettimeofday(&clck->start_, NULL)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGUTIL_API double*
aug_elapsed(aug_clock_t clck, double* sec)
{
    struct timeval now;
    if (-1 == aug_gettimeofday(&now, NULL)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }
    aug_tvsub(&now, &clck->start_);
    *sec = (double)now.tv_sec + ((double)now.tv_usec / 1000000.0);
    return sec;
}
