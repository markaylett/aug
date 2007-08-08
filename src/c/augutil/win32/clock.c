/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augsys/errinfo.h"
#include "augsys/windows.h"

#include <errno.h>  /* ENOMEM */
#include <stdlib.h> /* malloc() */

struct aug_clock_ {
    LARGE_INTEGER freq_, start_;
};

AUGUTIL_API aug_clock_t
aug_createclock(void)
{
    aug_clock_t clock = malloc(sizeof(struct aug_clock_));
	if (!clock) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

    if (!QueryPerformanceFrequency(&clock->freq_)
        || !QueryPerformanceCounter(&clock->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
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
    if (!QueryPerformanceCounter(&clock->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGUTIL_API int
aug_elapsed(aug_clock_t clock, double* secs)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }

    /* Ticks relative to start. */

    now.QuadPart -= clock->start_.QuadPart;
    *secs = (double)now.QuadPart / (double)clock->freq_.QuadPart;
    return 0;
}
