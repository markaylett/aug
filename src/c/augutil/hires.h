/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_HIRES_H
#define AUGUTIL_HIRES_H

/**
 * @file augutil/hires.h
 *
 * High resolution timer.
 */

#include "augutil/config.h"

#include "augext/mpool.h"

#include "augtypes.h"

typedef struct aug_hires_* aug_hires_t;

AUGUTIL_API aug_hires_t
aug_createhires(aug_mpool* mpool);

AUGUTIL_API void
aug_destroyhires(aug_hires_t hires);

/**
 * Reset hires to current time.
 */

AUGUTIL_API aug_result
aug_resethires(aug_hires_t hires);

/**
 * Elapsed time in seconds.
 */

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec);

#endif /* AUGUTIL_HIRES_H */
