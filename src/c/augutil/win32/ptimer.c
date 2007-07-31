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

AUGUTIL_API unsigned long
aug_ptimernow(aug_ptimer_t ptimer)
{
    LARGE_INTEGER ticks;
    unsigned long usec;

    QueryPerformanceCounter(&ticks);

    /* Ticks relative to start. */

    ticks.QuadPart -= ptimer->start_.QuadPart;

    /* Multiple before dividing. */

    usec = (unsigned long)ticks.QuadPart * 1000000;

    /* Ticks to microseconds. */

    usec /= (unsigned long)ptimer->freq_.QuadPart;

    return usec;
}
