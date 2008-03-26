/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#error deprecated
#ifndef AUGSYS_TIME_H
#define AUGSYS_TIME_H

/**
 * @file augsys/time.h
 *
 * Time functions.
 */

#include "augsys/config.h"

#if !defined(_REENTRANT)
# define _REENTRANT
#endif /* _REENTRANT */
#include <time.h>

#if !defined(_WIN32)
# include <sys/time.h>
#else /* _WIN32 */
# include <winsock2.h>
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif /* _WIN32 */

/**
 * Gets the current time in UTC.  tz is optional, if set, details of the local
 * timezone are returned.
 */

AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz);

/**
 * Some implementations of this function may modify the TZ environment
 * variable.  It should not, therefore, be considered thread-safe.
 */

AUGSYS_API time_t
aug_timegm(struct tm* tm);

AUGSYS_API time_t
aug_timelocal(struct tm* tm);

AUGSYS_API struct tm*
aug_gmtime(const time_t* clock, struct tm* res);

AUGSYS_API struct tm*
aug_localtime(const time_t* clock, struct tm* res);

AUGSYS_API struct timeval*
aug_mstotv(struct timeval* tv, unsigned ms);

AUGSYS_API unsigned
aug_tvtoms(const struct timeval* tv);

AUGSYS_API struct timeval*
aug_tvadd(struct timeval* dst, const struct timeval* src);

AUGSYS_API struct timeval*
aug_tvsub(struct timeval* dst, const struct timeval* src);

#endif /* AUGSYS_TIME_H */
