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
#define AUGCTX_BUILD
#include "augctx/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <stdlib.h>   /* getenv() */
#include <time.h>     /* tzset() */

#if defined(_WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h> /* GetTimeZoneInformation() */
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

AUGCTX_API int
aug_loglevel(void)
{
    const char* s = getenv("AUG_LOGLEVEL");
    return s ? atoi(s) : 4; /* AUG_LOGINFO */
}

#if !defined(_WIN32)
AUGCTX_API long*
aug_timezone(long* tz)
{
    tzset();
    *tz = timezone;
    return tz;
}
#else /* _WIN32 */
AUGCTX_API long*
aug_timezone(long* tz)
{
	TIME_ZONE_INFORMATION tzi;
    switch (GetTimeZoneInformation(&tzi)) {
    case TIME_ZONE_ID_INVALID:
    case TIME_ZONE_ID_UNKNOWN:
        return NULL;
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        break;
    }
    *tz = (tzi.Bias + tzi.StandardBias) * 60;
    return tz;
}
#endif /* _WIN32 */
