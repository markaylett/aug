/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <errno.h>
#include <pthread.h>
#include <stdlib.h> /* malloc() */

struct aug_mutex_ {
    pthread_mutex_t handle_;
};

AUGSYS_API aug_mutex_t
aug_createmutex(void)
{
    int err;
    aug_mutex_t mutex = malloc(sizeof(struct aug_mutex_));
    if (!mutex)
        return NULL;

    if ((err = pthread_mutex_init(&mutex->handle_, 0))) {
        errno = err;
        free(mutex);
        return NULL;
    }
	return mutex;
}

AUGSYS_API int
aug_freemutex(aug_mutex_t mutex)
{
    int ret = pthread_mutex_destroy(&mutex->handle_);
    free(mutex);
    if (ret) {
        errno = ret;
        ret = -1;
    }
    return ret;
}

AUGSYS_API int
aug_lockmutex(aug_mutex_t mutex)
{
    int ret = pthread_mutex_lock(&mutex->handle_);
    if (ret) {
        errno = ret;
        ret = -1;
    }
    return ret;
}

AUGSYS_API int
aug_unlockmutex(aug_mutex_t mutex)
{
    int ret = pthread_mutex_unlock(&mutex->handle_);
    if (ret) {
        errno = ret;
        ret = -1;
    }
    return ret;
}
