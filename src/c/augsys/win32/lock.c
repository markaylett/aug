/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/windows.h"

#include "augsys/errno.h"

#include <stdlib.h>

struct aug_mutex_ {
    CRITICAL_SECTION handle_;
};

AUG_EXTERN aug_mutex_t
aug_createmutex_(void)
{
    aug_mutex_t mutex = malloc(sizeof(struct aug_mutex_));
    if (!mutex) {
        errno = ENOMEM;
        return NULL;
    }

	/* In low memory situations, InitializeCriticalSection can raise a
       STATUS_NO_MEMORY exception. */

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
		InitializeCriticalSection(&mutex->handle_);
#if defined(_MSC_VER)
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		aug_setwin32errno(ERROR_NOT_ENOUGH_MEMORY);
        free(mutex);
		return NULL;
	}
#endif /* _MSC_VER */
	return mutex;
}

AUG_EXTERN int
aug_destroymutex_(aug_mutex_t mutex)
{
	DeleteCriticalSection(&mutex->handle_);
    free(mutex);
    return 0;
}

AUG_EXTERN int
aug_lockmutex_(aug_mutex_t mutex)
{
	/* In low memory situations, EnterCriticalSection can raise a
       STATUS_INVALID_HANDLE exception. */

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
		EnterCriticalSection(&mutex->handle_);
#if defined(_MSC_VER)
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		aug_setwin32errno(ERROR_INVALID_HANDLE);
		return -1;
	}
#endif /* _MSC_VER */
	return 0;
}

AUG_EXTERN int
aug_unlockmutex_(aug_mutex_t mutex)
{
	LeaveCriticalSection(&mutex->handle_);
	return 0;
}
