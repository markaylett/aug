/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"
#include "augsys/windows.h"

#include <stdlib.h>

struct aug_mutex_ {
    CRITICAL_SECTION handle_;
};

AUGSYS_API aug_mutex_t
aug_createmutex(void)
{
    aug_mutex_t mutex = malloc(sizeof(struct aug_mutex_));
    if (!mutex)
        return NULL;

	/* In low memory situations, InitializeCriticalSection can raise a
       STATUS_NO_MEMORY exception. */

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
		InitializeCriticalSection(&mutex->handle_);
#if defined(_MSC_VER)
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		aug_maperror(ERROR_NOT_ENOUGH_MEMORY);
        free(mutex);
		return NULL;
	}
#endif /* _MSC_VER */
	return mutex;
}

AUGSYS_API int
aug_freemutex(aug_mutex_t mutex)
{
	DeleteCriticalSection(&mutex->handle_);
    free(mutex);
    return 0;
}

AUGSYS_API int
aug_lockmutex(aug_mutex_t mutex)
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
		aug_maperror(ERROR_INVALID_HANDLE);
		return -1;
	}
#endif /* _MSC_VER */
	return 0;
}

AUGSYS_API int
aug_unlockmutex(aug_mutex_t mutex)
{
	LeaveCriticalSection(&mutex->handle_);
	return 0;
}
