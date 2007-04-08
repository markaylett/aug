/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <errno.h>
#include <sys/timeb.h>

AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz)
{
    struct _timeb tb;
    _ftime(&tb);

    if (tv) {
        tv->tv_sec = (long)tb.time;
        tv->tv_usec = tb.millitm * 1000;
    }

    if (tz) {
        tz->tz_minuteswest = tb.timezone;
        tz->tz_dsttime = tb.dstflag;
    }
    return 0;
}

AUGSYS_API time_t
aug_timegm(struct tm* tm)
{
    time_t ret;
    const char* tz = getenv("TZ");
    SetEnvironmentVariable("TZ", "");
    tzset();
    ret = mktime(tm);
    if (tz)
        SetEnvironmentVariable("TZ", tz);
    else
        SetEnvironmentVariable("TZ", NULL);
    tzset();
    if (ret == (time_t)-1)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);
    return ret;
}
