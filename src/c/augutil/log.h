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
#ifndef AUGUTIL_LOG_H
#define AUGUTIL_LOG_H

/**
 * @file augutil/log.h
 *
 * Common log format.
 */

#include "augutil/config.h"

#include "augext/clock.h"
#include "augext/log.h"
#include "augext/mpool.h"

/**
 * Get textual description of log label.
 *
 * @param level Log level.
 *
 * @return Textual description.
 */

AUGUTIL_API const char*
aug_loglabel(int level);

/**
 * Format log string.
 *
 * @param buf Output buffer.
 *
 * @param n In: size of @a buf.  Out: total number of characters copied
 *
 * @param clock Clock for timestamps.
 *
 * @param level Log level.
 *
 * @param format Printf-style format.
 *
 * @param args @a format arguments.
 *
 * @return See @ref TypesResult.
 */

AUGUTIL_API aug_result
aug_vformatlog(char* buf, size_t* n, aug_clock* clock, int level,
               const char* format, va_list args);

/**
 * @see aug_vformatlog().
 */

AUGUTIL_API aug_result
aug_formatlog(char* buf, size_t* n, aug_clock* clock, int level,
              const char* format, ...);

/**
 * Create a daemon logger.
 *
 * @param mpool Memory pool.
 *
 * @param clock Clock for timestamps.
 *
 * @return An interface to the daemon logger.
 */

AUGUTIL_API aug_log*
aug_createdaemonlog(aug_mpool* mpool, aug_clock* clock);

#endif /* AUGUTIL_LOG_H */
