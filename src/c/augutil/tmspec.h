/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
 * @return null on error.
 */

AUGUTIL_API struct aug_tmspec*
aug_strtmspec(struct aug_tmspec* tms, const char* s);

AUGUTIL_API struct tm*
aug_nexttime(struct tm* tm, const struct aug_tmspec* tms);

#endif /* AUGUTIL_TMSPEC_H */
