/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <stdlib.h> /* abort() */
#include <pthread.h>

static pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

AUG_EXTERNC aug_bool
aug_initlock_(void)
{
    /* Mutex is initialised statically. */

    return AUG_TRUE;
}

AUGCTX_API void
aug_lock(void)
{
    if (0 != pthread_mutex_lock(&mutex_))
        abort();
}

AUGCTX_API void
aug_unlock(void)
{
    if (0 != pthread_mutex_unlock(&mutex_))
        abort();
}
