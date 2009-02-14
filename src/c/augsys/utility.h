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
#ifndef AUGSYS_UTILITY_H
#define AUGSYS_UTILITY_H

#include "augsys/config.h"
#include "augsys/types.h"

/**
 * Scramble or unscramble @a dst buffer.
 *
 * Useful for hiding memory contents, but not true encryption.
 *
 * @param dst The buffer.
 *
 * @param size Size of @a dst buffer.
 *
 * @return @a dst buffer.
 */

AUGSYS_API void*
aug_memfrob(void* dst, size_t size);

/**
 * Get next process-unique id.
 *
 * Thread-safe.  Cannot fail.  Will loop when #INT_MAX is reached.
 *
 * @return Next id.
 */

AUGSYS_API unsigned
aug_nextid(void);

AUGSYS_API long
aug_rand(void);

AUGSYS_API void
aug_srand(unsigned seed);

/**
 * @return Thread identifier, or 0 if the library has been compiled without
 * thread support.
 */

AUGSYS_API unsigned
aug_threadid(void);

#endif /* AUGSYS_UTILITY_H */
