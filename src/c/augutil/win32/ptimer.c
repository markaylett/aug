/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"
#include "augsys/windows.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_ptimer_ {
    LARGE_INTEGER freq_, start_;
};

AUGUTIL_API aug_ptimer_t
aug_createptimer(void)
{
    aug_ptimer_t ptimer = malloc(sizeof(struct aug_ptimer_));
	if (!ptimer) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (!QueryPerformanceFrequency(&ptimer->freq_)
        || !QueryPerformanceCounter(&ptimer->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
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
    if (!QueryPerformanceCounter(&ptimer->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGUTIL_API int
aug_elapsed(aug_ptimer_t ptimer, double* secs)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Ticks relative to start. */

    now.QuadPart -= ptimer->start_.QuadPart;
    *secs = (double)now.QuadPart / (double)ptimer->freq_.QuadPart;
    return 0;
}
