/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
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
