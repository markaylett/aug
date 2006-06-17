/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <errno.h>

AUGSYS_API int
aug_gettimeofday(struct timeval* tv, struct timezone* tz)
{
    if (-1 == gettimeofday(tv, tz)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}
