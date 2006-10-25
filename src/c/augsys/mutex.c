/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mutex.h"

#include "augsys/errinfo.h"
#include "augsys/lock.h"

#include <errno.h>

static const char rcsid[] = "$Id$";

AUGSYS_API aug_mutex_t
aug_createmutex(void)
{
    aug_mutex_t mutex = aug_createmutex_();
    if (!mutex)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return mutex;
}

AUGSYS_API int
aug_freemutex(aug_mutex_t mutex)
{
    int ret = aug_freemutex_(mutex);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

AUGSYS_API int
aug_lockmutex(aug_mutex_t mutex)
{
    int ret = aug_lockmutex_(mutex);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}

AUGSYS_API int
aug_unlockmutex(aug_mutex_t mutex)
{
    int ret = aug_unlockmutex_(mutex);
    if (-1 == ret)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
    return ret;
}
