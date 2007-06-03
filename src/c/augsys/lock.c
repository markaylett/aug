/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/lock.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include <errno.h>
#include <stdlib.h> /* NULL */

#if ENABLE_THREADS

# if !defined(_WIN32)
#  include "augsys/posix/lock.c"
# else /* _WIN32 */
#  include "augsys/win32/lock.c"
# endif /* _WIN32 */

static aug_mutex_t mutex_ = NULL;

AUG_EXTERNC int
aug_initlock_(void)
{
    if (mutex_) {
        errno = EINVAL;
        return -1;
    }

    if (!(mutex_ = aug_createmutex_()))
        return -1;

    return 0;
}

AUG_EXTERNC int
aug_termlock_(void)
{
    aug_mutex_t tmp = mutex_;

    if (!tmp) {
        errno = EINVAL;
        return -1;
    }

    mutex_ = NULL;
    return aug_destroymutex_(tmp);
}

/* If these locking functions fail, all bets are off. */

AUGSYS_API void
aug_lock(void)
{
    if (!mutex_ || -1 == aug_lockmutex_(mutex_))
        abort();
}

AUGSYS_API void
aug_unlock(void)
{
    if (!mutex_ || -1 == aug_unlockmutex_(mutex_))
        abort();
}

#else /* !ENABLE_THREADS */

struct aug_mutex_ { char dummy_; };

AUG_EXTERNC aug_mutex_t
aug_createmutex_(void)
{
	return (aug_mutex_t)~0;
}

AUG_EXTERNC int
aug_destroymutex_(aug_mutex_t mutex)
{
    return 0;
}

AUG_EXTERNC int
aug_lockmutex_(aug_mutex_t mutex)
{
	return 0;
}

AUG_EXTERNC int
aug_unlockmutex_(aug_mutex_t mutex)
{
	return 0;
}

AUG_EXTERNC int
aug_initlock_(void)
{
    return 0;
}

AUG_EXTERNC int
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

#endif /* !ENABLE_THREADS */
