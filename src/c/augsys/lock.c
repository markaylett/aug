/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/lock.h"

static const char rcsid[] = "$Id:$";

#include "augsys/errno.h"
#include "augsys/mutex.h"

#include <stdlib.h> /* NULL */

#if !defined(_MT)

AUGSYS_EXTERN int
aug_initlock_(void)
{
    return 0;
}

AUGSYS_EXTERN int
aug_termlock_(void)
{
    return 0;
}

AUGSYS_API void
aug_lock(void)
{
}

AUGSYS_API void
aug_unlock(void)
{
}

#else /* _MT */

static aug_mutex_t mutex_ = NULL;

AUGSYS_EXTERN int
aug_initlock_(void)
{
    if (mutex_) {
        errno = EINVAL;
        return -1;
    }

    if (!(mutex_ = aug_createmutex()))
        return -1;

    return 0;
}

AUGSYS_EXTERN int
aug_termlock_(void)
{
    aug_mutex_t tmp = mutex_;

    if (!tmp) {
        errno = EINVAL;
        return -1;
    }

    mutex_ = NULL;
    return aug_freemutex(tmp);
}

AUGSYS_API void
aug_lock(void)
{
    if (!mutex_ || -1 == aug_lockmutex(mutex_))
        abort();
}

AUGSYS_API void
aug_unlock(void)
{
    if (!mutex_ || -1 == aug_unlockmutex(mutex_))
        abort();
}

#endif /* _MT */
