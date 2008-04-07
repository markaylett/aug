/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <stdlib.h>   /* getenv() */
#include <time.h>     /* tzset() */

#if defined(AUG_WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h> /* GetTimeZoneInformation() */
# define vsnprintf _vsnprintf
#endif /* AUG_WIN32 */

AUGCTX_API int
aug_loglevel(void)
{
    const char* s = getenv("AUG_LOGLEVEL");
    return s ? atoi(s) : 4; /* AUG_LOGINFO */
}

#if !defined(AUG_WIN32)
AUGCTX_API long*
aug_timezone(long* tz)
{
    tzset();
    *tz = timezone;
    return tz;
}
#else /* AUG_WIN32 */
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
#endif /* AUG_WIN32 */
