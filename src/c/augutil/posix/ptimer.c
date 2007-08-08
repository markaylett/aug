/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */
#include <time.h>

struct aug_ptimer_ {
    clock_t start_;
};

AUGUTIL_API aug_ptimer_t
aug_createptimer(void)
{
    aug_ptimer_t ptimer = malloc(sizeof(struct aug_ptimer_));
	if (!ptimer) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (-1 == (ptimer->start_ = clock())) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
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
    if (-1 == (ptimer->start_ = clock())) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGUTIL_API int
aug_elapsed(aug_ptimer_t ptimer, double* secs)
{
    clock_t now = clock();
    if (-1 == now) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    now -= ptimer->start_;
    *secs = (double)now / (double)CLOCKS_PER_SEC;
    return 0;
}
