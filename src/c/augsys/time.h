/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TIME_H
#define AUGSYS_TIME_H

#include "augsys/config.h"

#define _REENTRANT
#include <time.h>

#if !defined(_WIN32)
# include <sys/time.h>
#else // _WIN32
# include <winsock2.h>
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif // _WIN32

AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz);

AUGSYS_API struct tm*
aug_localtime(const time_t* clock, struct tm* res);

#endif /* AUGSYS_TIME_H */
