/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <errno.h>

AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz)
{
    if (-1 == gettimeofday(tv, tz)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGSYS_API time_t
aug_timegm(struct tm* tm)
{
#if HAVE_TIMEGM

    time_t ret = timegm(tm);

#else /* !HAVE_TIMEGM */

    /* TODO: can this function be made thread-safe? */

    time_t ret;
    const char* tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    ret = mktime(tm);
    if (tz)
        setenv("TZ", tz, 1);
    else
        unsetenv("TZ");
    tzset();

#endif /* !HAVE_TIMEGM */

    if (ret == (time_t)-1)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);
    return ret;
}
