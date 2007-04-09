/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/time.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"

#if !defined(_WIN32)
# include "augsys/posix/time.c"
#else /* _WIN32 */
# include "augsys/win32/time.c"
#endif /* _WIN32 */

AUGSYS_API time_t
aug_timelocal(struct tm* tm)
{
    time_t ret = mktime(tm);
    if (ret == (time_t)-1)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
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
    aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
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
    aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
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
