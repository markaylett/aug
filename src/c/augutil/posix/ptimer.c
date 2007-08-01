/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_ptimer_ {
    struct timeval start_;
};

AUGUTIL_API aug_ptimer_t
aug_createptimer(void)
{
    aug_ptimer_t ptimer = malloc(sizeof(struct aug_ptimer_));
	if (!ptimer) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (-1 == aug_gettimeofday(&ptimer->start_, NULL)) {
        free(ptimer);
        return NULL;
    }

	return ptimer;
}

AUGUTIL_API int
aug_destroyptimer(aug_ptimer_t ptimer)
{
	if (ptimer)
		free(ptimer);
    return 0;
}

AUGUTIL_API int
aug_resetptimer(aug_ptimer_t ptimer)
{
    return aug_gettimeofday(&ptimer->start_, NULL);
}

AUGUTIL_API struct timeval*
aug_elapsed(aug_ptimer_t ptimer, struct timeval* tv)
{
    if (-1 == aug_gettimeofday(tv, NULL))
        return NULL;

    aug_tvsub(tv, &ptimer->start_);
    return tv;
}
