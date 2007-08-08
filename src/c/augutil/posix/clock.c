/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */
#include <time.h>

struct aug_clock_ {
    clock_t start_;
};

AUGUTIL_API aug_clock_t
aug_createclock(void)
{
    aug_clock_t clock = malloc(sizeof(struct aug_clock_));
	if (!clock) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (-1 == (clock->start_ = clock())) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        free(clock);
        return NULL;
    }

	return clock;
}

AUGUTIL_API int
aug_destroyclock(aug_clock_t clock)
{
	if (clock)
		free(clock);
    return 0;
}

AUGUTIL_API int
aug_resetclock(aug_clock_t clock)
{
    if (-1 == (clock->start_ = clock())) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGUTIL_API int
aug_elapsed(aug_clock_t clock, double* secs)
{
    clock_t now = clock();
    if (-1 == now) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    now -= clock->start_;
    *secs = (double)now / (double)CLOCKS_PER_SEC;
    return 0;
}
