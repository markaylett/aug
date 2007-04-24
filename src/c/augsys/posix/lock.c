/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <errno.h>
#include <pthread.h>
#include <stdlib.h> /* malloc() */

struct aug_mutex_ {
    pthread_mutex_t handle_;
};

AUG_EXTERN aug_mutex_t
aug_createmutex_(void)
{
    aug_mutex_t mutex = malloc(sizeof(struct aug_mutex_));
    if (!mutex) {
        errno = ENOMEM;
        return NULL;
    }

    if (0 != (errno = pthread_mutex_init(&mutex->handle_, 0))) {
        free(mutex);
        return NULL;
    }
	return mutex;
}

AUG_EXTERN int
aug_destroymutex_(aug_mutex_t mutex)
{
    errno = pthread_mutex_destroy(&mutex->handle_);
    free(mutex);
    return 0 == errno ? 0 : -1;
}

AUG_EXTERN int
aug_lockmutex_(aug_mutex_t mutex)
{
    errno = pthread_mutex_lock(&mutex->handle_);
    return 0 == errno ? 0 : -1;
}

AUG_EXTERN int
aug_unlockmutex_(aug_mutex_t mutex)
{
    errno = pthread_mutex_unlock(&mutex->handle_);
    return 0 == errno ? 0 : -1;
}
