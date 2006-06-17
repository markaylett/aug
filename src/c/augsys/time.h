/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TIME_H
#define AUGSYS_TIME_H

#include "augsys/config.h"

#if !defined(_REENTRANT)
# define _REENTRANT
#endif /* _REENTRANT */
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

AUGSYS_API struct timeval*
aug_mstotv(struct timeval* tv, unsigned int ms);

AUGSYS_API unsigned int
aug_tvtoms(const struct timeval* tv);

AUGSYS_API struct timeval*
aug_tvadd(struct timeval* dst, const struct timeval* src);

AUGSYS_API struct timeval*
aug_tvsub(struct timeval* dst, const struct timeval* src);

#endif /* AUGSYS_TIME_H */
