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
#include "augctx/utility.h" /* aug_timezone() */

AUGSYS_API time_t
aug_timegm(struct tm* tm)
{
    time_t ret;

#if HAVE_TIMEGM

    if ((time_t)-1 == (ret = timegm(tm)))
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);

#else /* !HAVE_TIMEGM */

    struct tm gm = { 0 };
    long tz;

    gm.tm_sec = tm->tm_sec;
    gm.tm_min = tm->tm_min;
    gm.tm_hour = tm->tm_hour;
    gm.tm_mday = tm->tm_mday;
    gm.tm_mon = tm->tm_mon;
    gm.tm_year = tm->tm_year;
    gm.tm_isdst = 0; /* No daylight adjustment. */

    if ((time_t)-1 == (ret = mktime(&gm))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);
        return ret;
    }

    /* mktime() assumes localtime; adjust for gmt. */

    ret -= *aug_timezone(&tz);

#endif /* !HAVE_TIMEGM */

    return ret;
}

AUGSYS_API time_t
aug_timelocal(struct tm* tm)
{
    time_t ret = mktime(tm);
    if (ret == (time_t)-1)
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);
    return ret;
}

AUGSYS_API struct tm*
aug_gmtime(const time_t* clock, struct tm* res)
{
    struct tm* ret;

    /* Although there is no documented failure condition for the gmtime_r
       function, provisions are made for interpreting a NULL return as an
       error condition. */

    errno = 0;

#if HAVE_LOCALTIME_R

    if (!(ret = gmtime_r(clock, res)))
        goto fail;

#elif defined(_WIN32)

    /* Note: On windows, the gmtime function is implemented using thread-local
       storage - making it thread safe (but not re-entrant). */

    if (!(ret = gmtime(clock)))
        goto fail;

    memcpy(res, ret, sizeof(*res));

#else /* !HAVE_LOCALTIME_R && !_WIN32 */
# error "gmtime_r is required on non-Windows platform"
#endif /* !HAVE_LOCALTIME_R && !_WIN32 */

    return res;

 fail:
    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, 0 == errno
                        ? EINVAL : errno);
    return NULL;
}

AUGSYS_API struct tm*
aug_localtime(const time_t* clock, struct tm* res)
{
    struct tm* ret;

    /* Although there is no documented failure condition for the localtime_r
       function, provisions are made for interpreting a NULL return as an
       error condition. */

    errno = 0;

#if HAVE_LOCALTIME_R

    if (!(ret = localtime_r(clock, res)))
        goto fail;

#elif defined(_WIN32)

    /* Note: On windows, the localtime function is implemented using
       thread-local storage - making it thread safe (but not re-entrant). */

    if (!(ret = localtime(clock)))
        goto fail;

    memcpy(res, ret, sizeof(*res));

#else /* !HAVE_LOCALTIME_R && !_WIN32 */
# error "localtime_r is required on non-Windows platform"
#endif /* !HAVE_LOCALTIME_R && !_WIN32 */

    return res;

 fail:
    aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, 0 == errno
                        ? EINVAL : errno);
    return NULL;
}

AUGSYS_API struct timeval*
aug_mstotv(struct timeval* tv, unsigned ms)
{
    tv->tv_sec = ms / 1000;
    tv->tv_usec = (ms % 1000) * 1000;
    return tv;
}

AUGSYS_API unsigned
aug_tvtoms(const struct timeval* tv)
{
    unsigned ms = tv->tv_sec * 1000;
    ms += (tv->tv_usec + 500) / 1000;
    return ms;
}

AUGSYS_API struct timeval*
aug_tvadd(struct timeval* dst, const struct timeval* src)
{
    dst->tv_sec += src->tv_sec;
    dst->tv_usec += src->tv_usec;

    if (1000000 < dst->tv_usec) {
        ++dst->tv_sec;
        dst->tv_usec -= 1000000;
    }
    return dst;
}

AUGSYS_API struct timeval*
aug_tvsub(struct timeval* dst, const struct timeval* src)
{
    dst->tv_sec -= src->tv_sec;
    dst->tv_usec -= src->tv_usec;

    if (dst->tv_usec < 0) {
        --dst->tv_sec;
        dst->tv_usec += 1000000;
    }
    return dst;
}
