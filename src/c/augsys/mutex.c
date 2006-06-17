/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/mutex.h"

#if !defined(_MT)

struct aug_mutex_ { char dummy_; };

AUGSYS_API aug_mutex_t
aug_createmutex(void)
{
	return (aug_mutex_t)~0;
}

AUGSYS_API int
aug_freemutex(aug_mutex_t mutex)
{
    return 0;
}

AUGSYS_API int
aug_lockmutex(aug_mutex_t mutex)
{
	return 0;
}

AUGSYS_API int
aug_unlockmutex(aug_mutex_t mutex)
{
	return 0;
}

#else /* _MT */
# if !defined(_WIN32)
#  include "augsys/posix/mutex.c"
# else /* _WIN32 */
#  include "augsys/win32/mutex.c"
# endif /* _WIN32 */
#endif /* _MT */
