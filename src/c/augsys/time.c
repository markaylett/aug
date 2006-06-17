/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/time.h"

#if HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#if !defined(_WIN32)
# include "augsys/posix/time.c"
#else /* _WIN32 */
# include "augsys/win32/time.c"
#endif /* _WIN32 */

#include <errno.h>

AUGSYS_API struct tm*
aug_localtime(const time_t* clock, struct tm* res)
{
    struct tm* ret;

    /* Although there is no documented failure condition for the localtime_r
       function, provisions are made for interpreting a NULL return as an error
       condition. */

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
# error localtime_r is required for non-Windows platform
#endif /* !HAVE_LOCALTIME_R && !_WIN32 */

    return ret;

 fail:
    if (0 == errno)
        errno = EINVAL;

    return NULL;
}
