/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_TMSPEC_H
#define AUGUTIL_TMSPEC_H

/**
 * @file augutil/tmspec.h
 *
 * Time specifications.
 */

#include "augutil/config.h"

struct tm;

struct aug_tmspec {
    int min_, hour_, mday_, mon_, wday_;
};

/**
 * Populate structure from specification.
 *
 * Specifications can contain the following time components:
 *
 * @li m - month
 * @li d - day of month
 * @li H - hour of day
 * @li M - minute of hour
 *
 * So that, for example, "17H0M" would be taken to mean "daily, at 5 o'clock".
 *
 * @param tms Structure to be populated.
 *
 * @param s Specification.
 *
 * @return NULL on error.
 */

AUGUTIL_API struct aug_tmspec*
aug_strtmspec(struct aug_tmspec* tms, const char* s);

AUGUTIL_API struct tm*
aug_nexttime(struct tm* tm, const struct aug_tmspec* tms);

#endif /* AUGUTIL_TMSPEC_H */
