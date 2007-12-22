/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CLOCK_H
#define AUGUTIL_CLOCK_H

/**
 * @file augutil/clock.h
 *
 * High resolution timer.
 */

#include "augutil/config.h"

typedef struct aug_clock_* aug_clock_t;

AUGUTIL_API aug_clock_t
aug_createclock(void);

AUGUTIL_API int
aug_destroyclock(aug_clock_t clck);

AUGUTIL_API int
aug_resetclock(aug_clock_t clck);

/**
 * Elapsed time in seconds.
 */

AUGUTIL_API double*
aug_elapsed(aug_clock_t clck, double* sec);

#endif /* AUGUTIL_CLOCK_H */
