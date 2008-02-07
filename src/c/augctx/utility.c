/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/types.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>   /* getenv() */
#include <string.h>
#include <time.h>     /* tzset() */

#if defined(_WIN32)
# include <windows.h> /* GetTimeZoneInformation() */
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

AUGCTX_API size_t
aug_strlcpy(char* dst, const char* src, size_t size)
{
    /* Thanks to Dan Cross for this public domain implementation of
       strlcpy(). */

    size_t len, srclen;
    srclen = strlen(src);
    if (--size <= 0) return(srclen);
    len = (size < srclen) ? size : srclen;
    memmove(dst, src, len);
    dst[len] = '\0';
    return(srclen);
}

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args)
{
    int ret;

    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));
    errinfo->line_ = line;
    aug_strlcpy(errinfo->src_, src, sizeof(errinfo->src_));
    errinfo->num_ = num;

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = vsnprintf(errinfo->desc_, sizeof(errinfo->desc_), format, args);
    AUG_SNTRUNCF(errinfo->desc_, sizeof(errinfo->desc_), ret);

    if (ret < 0)
        aug_strlcpy(errinfo->desc_, "no message - bad format",
                    sizeof(errinfo->desc_));
}

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    aug_vseterrinfo(errinfo, file, line, src, num, format, args);
    va_end(args);
}

AUGCTX_API int
aug_loglevel(void)
{
    const char* s = getenv("AUG_LOGLEVEL");
    return s ? atoi(s) : AUG_LOGINFO;
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
