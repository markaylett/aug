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
#ifndef AUGCTX_UTILITY_H
#define AUGCTX_UTILITY_H

#include "augctx/config.h"

#include <stdarg.h>
#include <stddef.h>

#define aug_die_(file, line, what) \
(fprintf(stderr, "%s:%d: %s\n", file, line, what), fflush(NULL), exit(1))

#define aug_die(what) \
aug_die_(__FILE__, __LINE__, what)

#define aug_check(expr) \
(expr) ? (void)0 : aug_die("check [" #expr "] failed.")

/**
 * Get the default log-level.
 *
 * The log-level specified by the "AUG_LOGLEVEL" environment variable, or
 * #AUG_LOGINFO if not set.
 *
 * @return The log-level.
 */

AUGCTX_API unsigned
aug_loglevel(void);

/**
 * Get timezone offset.
 *
 * Attempt to determine current timezone as seconds west of coordinated
 * universal time.  If a process intends to call chroot(), it should call this
 * function first and cache the result: timezone information may be
 * inaccessible to a jailed process.
 *
 * @param tz Output variable to which timezone will be written.
 *
 * @return The @a tz argument, or null on error.
 */

AUGCTX_API long*
aug_timezone(long* tz);

#endif /* AUGCTX_UTILITY_H */
