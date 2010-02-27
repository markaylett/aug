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
#define AUGSYS_BUILD
#include "augsys/time.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"

#if !HAVE_TIMEGM
static long
timezone_(void)
{
    aug_clock* clock = aug_getclock(aug_tlx);
    long tz = aug_gettimezone(clock);
    aug_release(clock);
	return tz;
}
#endif /* !HAVE_TIMEGM */

AUGSYS_API aug_rlong
aug_timegm(struct tm* tm)
{
    time_t gmt;

#if HAVE_TIMEGM

    if ((time_t)-1 == (gmt = timegm(tm)))
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);

#else /* !HAVE_TIMEGM */

    struct tm gm = { 0 };

    gm.tm_sec = tm->tm_sec;
    gm.tm_min = tm->tm_min;
    gm.tm_hour = tm->tm_hour;
    gm.tm_mday = tm->tm_mday;
    gm.tm_mon = tm->tm_mon;
    gm.tm_year = tm->tm_year;
    gm.tm_isdst = 0; /* No daylight adjustment. */

    if ((time_t)-1 == (gmt = mktime(&gm))) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__,
                          0 == errno ? EINVAL : errno);
        return -1;
    }

    /* mktime() assumes localtime; adjust for gmt. */

    gmt -= timezone_();

#endif /* !HAVE_TIMEGM */

    return gmt;
}

AUGSYS_API aug_rlong
aug_timelocal(struct tm* tm)
{
    time_t gmt = mktime(tm);
    if (gmt == (time_t)-1) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__,
                          0 == errno ? EINVAL : errno);
        return -1;
    }
    return gmt;
}

AUGSYS_API struct tm*
aug_gmtime(const aug_time* clock, struct tm* res)
{
    time_t gmt = *clock;
    struct tm* ret;

    /* Although there is no documented failure condition for the gmtime_r
       function, provisions are made for interpreting a NULL return as an
       error condition. */

    errno = 0;

#if HAVE_LOCALTIME_R

    if (!(ret = gmtime_r(&gmt, res)))
        goto fail;

#elif defined(_WIN32)

    /* Note: On windows, the gmtime function is implemented using thread-local
       storage - making it thread safe (but not re-entrant). */

    if (!(ret = gmtime(&gmt)))
        goto fail;

    memcpy(res, ret, sizeof(*res));

#else /* !HAVE_LOCALTIME_R && !_WIN32 */
# error "gmtime_r is required on non-Windows platform"
#endif /* !HAVE_LOCALTIME_R && !_WIN32 */

    return res;

 fail:
    aug_setposixerror(aug_tlx, __FILE__, __LINE__,
                      0 == errno ? EINVAL : errno);
    return NULL;
}

AUGSYS_API struct tm*
aug_localtime(const aug_time* clock, struct tm* res)
{
    time_t gmt = *clock;
    struct tm* ret;

    /* Although there is no documented failure condition for the localtime_r
       function, provisions are made for interpreting a NULL return as an
       error condition. */

    errno = 0;

#if HAVE_LOCALTIME_R

    if (!(ret = localtime_r(&gmt, res)))
        goto fail;

#elif defined(_WIN32)

    /* Note: On windows, the localtime function is implemented using
       thread-local storage - making it thread safe (but not re-entrant). */

    if (!(ret = localtime(&gmt)))
        goto fail;

    memcpy(res, ret, sizeof(*res));

#else /* !HAVE_LOCALTIME_R && !_WIN32 */
# error "localtime_r is required on non-Windows platform"
#endif /* !HAVE_LOCALTIME_R && !_WIN32 */

    return res;

 fail:
    aug_setposixerror(aug_tlx, __FILE__, __LINE__,
                      0 == errno ? EINVAL : errno);
    return NULL;
}

AUGSYS_API struct aug_timeval*
aug_mstotv(unsigned ms, struct aug_timeval* tv)
{
    tv->tv_sec = ms / 1000;
    tv->tv_usec = (ms % 1000) * 1000;
    return tv;
}

AUGSYS_API unsigned
aug_tvtoms(const struct aug_timeval* tv)
{
    unsigned ms = tv->tv_sec * 1000;
    ms += (tv->tv_usec + 500) / 1000;
    return ms;
}

AUGSYS_API struct aug_timeval*
aug_tvadd(struct aug_timeval* dst, const struct aug_timeval* src)
{
    dst->tv_sec += src->tv_sec;
    dst->tv_usec += src->tv_usec;

    if (1000000 < dst->tv_usec) {
        ++dst->tv_sec;
        dst->tv_usec -= 1000000;
    }
    return dst;
}

AUGSYS_API struct aug_timeval*
aug_tvsub(struct aug_timeval* dst, const struct aug_timeval* src)
{
    if (timercmp(dst, src, <)) {

        dst->tv_sec = 0;
        dst->tv_usec = 0;

    } else {

        dst->tv_sec -= src->tv_sec;
        dst->tv_usec -= src->tv_usec;

        if (dst->tv_usec < 0) {
            --dst->tv_sec;
            dst->tv_usec += 1000000;
        }
    }
    return dst;
}
