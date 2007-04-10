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
    TIME_ZONE_INFORMATION tz;
    time_t ret = mktime(tm);

    if (ret == (time_t)-1) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, 0 == errno
                            ? EINVAL : errno);
        return ret;
    }

    switch(GetTimeZoneInformation(&tz)) {
    case TIME_ZONE_ID_INVALID:
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, GetLastError());
        ret = (time_t)-1;
        break;
    case TIME_ZONE_ID_UNKNOWN:
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("timezone id unknown"));
        ret = (time_t)-1;
        break;
    case TIME_ZONE_ID_STANDARD:
        ret -= (tz.Bias + tz.StandardBias) * 60;
        break;
    case TIME_ZONE_ID_DAYLIGHT:
        ret -= (tz.Bias + tz.DaylightBias) * 60;
        break;
    }

    return ret;
}
