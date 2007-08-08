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
    aug_clock_t clck = malloc(sizeof(struct aug_clock_));
	if (!clck) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (-1 == (clck->start_ = clock())) {
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
    if (-1 == (clck->start_ = clock())) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGUTIL_API int
aug_elapsed(aug_clock_t clck, double* secs)
{
    clock_t now = clock();
    if (-1 == now) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    now -= clck->start_;
    *secs = (double)now / (double)CLOCKS_PER_SEC;
    return 0;
}
