/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

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
