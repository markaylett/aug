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
    aug_clock_t clck = malloc(sizeof(struct aug_clock_));
    if (!clck) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!QueryPerformanceFrequency(&clck->freq_)
        || !QueryPerformanceCounter(&clck->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
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
    if (!QueryPerformanceCounter(&clck->start_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return -1;
    }
    return 0;
}

AUGUTIL_API double*
aug_elapsed(aug_clock_t clck, double* sec)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        return NULL;
    }

    /* Ticks relative to start. */

    now.QuadPart -= clck->start_.QuadPart;
    *sec = (double)now.QuadPart / (double)clck->freq_.QuadPart;
    return sec;
}
