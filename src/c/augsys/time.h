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
#ifndef AUGSYS_TIME_H
#define AUGSYS_TIME_H

/**
 * @file augsys/time.h
 *
 * Time functions.
 */

#include "augsys/config.h"

#include "augtypes.h"

#if !defined(_REENTRANT)
# define _REENTRANT
#endif /* _REENTRANT */
#include <time.h>

#if !defined(_WIN32)
# include <sys/time.h>
#else /* _WIN32 */
# include <winsock2.h>
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif /* _WIN32 */

/**
 * Some implementations of this function may modify the TZ environment
 * variable.  It should not, therefore, be considered thread-safe.
 */

AUGSYS_API aug_time
aug_timegm(struct tm* tm);

AUGSYS_API aug_time
aug_timelocal(struct tm* tm);

AUGSYS_API struct tm*
aug_gmtime(const aug_time* clock, struct tm* res);

AUGSYS_API struct tm*
aug_localtime(const aug_time* clock, struct tm* res);

AUGSYS_API struct aug_timeval*
aug_mstotv(unsigned ms, struct aug_timeval* tv);

AUGSYS_API unsigned
aug_tvtoms(const struct aug_timeval* tv);

AUGSYS_API struct aug_timeval*
aug_tvadd(struct aug_timeval* dst, const struct aug_timeval* src);

/**
 * Subtract @a src from @a dst.  If @a dst is greater than @a src, then the
 * result is zero.
 */

AUGSYS_API struct aug_timeval*
aug_tvsub(struct aug_timeval* dst, const struct aug_timeval* src);

#endif /* AUGSYS_TIME_H */
